//---------------------------------------------------------------------------------------------------------------------
//  DVSAL
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2020 - Marco Montes Grova (a.k.a. mgrova) marrcogrova@gmail.com 
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#include <dvsal/processors/mapping/MapperEMVS.h>

#include <opencv2/highgui/highgui.hpp>
#include <pcl/io/pcd_io.h>

#include <chrono>

std::vector<dv::Event> file2EventsVector(std::string _file){
  std::ifstream datasetFile;
  datasetFile.open(_file);
  
  std::vector<dv::Event> events;      
  while(!datasetFile.eof()){
    std::string line;
    std::getline(datasetFile, line);  

    std::istringstream iss(line);
    float timestamp;
    int x, y, pol;
    iss >> timestamp >> x >> y >> pol;

    dv::Event event(static_cast<int64_t>(timestamp * 1000000) , static_cast<int16_t>(x) , static_cast<int16_t>(y) , static_cast<uint8_t>(pol));
    events.push_back(event);
  }
  datasetFile.close();

  return events;
}

std::map<float, Eigen::Matrix4f> file2PosesMap(std::string _file){
  std::ifstream datasetFile;
  datasetFile.open(_file);
  
  std::map<float, Eigen::Matrix4f> poses;      
  while(!datasetFile.eof()){
    std::string line;
    std::getline(datasetFile, line);  

    std::istringstream iss(line);
    float timestamp; //sec
    float px, py, pz ,qx ,qy ,qz ,qw;
    iss >> timestamp >> px >> py >> pz >> qx >> qy >> qz >> qw;

    Eigen::Vector3f position(px,py,pz);
    Eigen::Quaternionf orientation(qw,qx,qy,qz);
    Eigen::Matrix4f T = Eigen::Matrix4f::Identity();
    T.block<3,1>(0,3) = position;
    T.block<3,3>(0,0) = orientation.toRotationMatrix();

    poses[timestamp] = T;
  }
  datasetFile.close();

  return poses;
}


int main(int argc, char** argv)
{
  
  auto events = file2EventsVector("/home/grvc/Downloads/slider_depth/events.txt");
  auto poses  = file2PosesMap("/home/grvc/Downloads/slider_depth/groundtruth.txt");

  std::cout << "Events size: " << events.size() << " poses size: " << poses.size() << std::endl;

  // Use linear interpolation to compute the camera pose for each event
  LinearTrajectory trajectory = LinearTrajectory(poses);

  // Set the position of the reference view in the middle of the trajectory
  Eigen::Matrix4f T0_= Eigen::Matrix4f::Identity();
  Eigen::Matrix4f T1_= Eigen::Matrix4f::Identity();
  float t0_, t1_;
  trajectory.getFirstControlPose(&T0_, &t0_);
  trajectory.getLastControlPose(&T1_, &t1_);
  Eigen::Matrix4f T_w_rv = Eigen::Matrix4f::Identity();


  trajectory.getPoseAt(0.5 * (t0_ + t1_), T_w_rv);
  Eigen::Matrix4f T_rv_w = T_w_rv.inverse();

  std::cout << T_rv_w << "\n";
  

  // Initialize the DSI
  size_t dimX      = 0;
  size_t dimY      = 0;
  size_t dimZ      = 100;
  float minDepth   = 0.3;
  float maxDepth   = 5.0; // Number of depth planes should be <= 256
  float fovDegrees = 0.0;
  EMVS::ShapeDSI dsi_shape(dimX, dimY, dimZ, minDepth, maxDepth, fovDegrees);

  std::vector<float> cameraParameters = {335.41946 , 335.35294 , 129.92466 , 99.18643 , 240.0 , 180.0}; // fx fy cx cy width height
  EMVS::MapperEMVS mapper(cameraParameters, dsi_shape);

  // 1. Back-project events into the DSI
  // std::chrono::high_resolution_clock::time_point t_start_dsi = std::chrono::high_resolution_clock::now();
  // mapper.evaluateDSI(events, trajectory, T_rv_w);
  // std::chrono::high_resolution_clock::time_point t_end_dsi = std::chrono::high_resolution_clock::now();
  // auto duration_dsi = std::chrono::duration_cast<std::chrono::milliseconds>(t_end_dsi - t_start_dsi ).count();
  // std::cout << "Time to evaluate DSI: " << duration_dsi << " milliseconds \n";
  // std::cout << "Number of events processed: " << events.size() << " events \n";
  // std::cout << "Number of events processed per second: " << static_cast<float>(events.size()) / (1000.f * static_cast<float>(duration_dsi)) << " Mev/s \n";
  
  // std::cout << "Mean square = " << mapper.dsi_.computeMeanSquare();

  // // Write the DSI (3D voxel grid) to disk
  // // mapper.dsi_.writeGridNpy("dsi.npy");


  // // 2. Extract semi-dense depth map from DSI
  // EMVS::OptionsDepthMap opts_depth_map;
  // opts_depth_map.adaptive_threshold_kernel_size_ = 5;
  // opts_depth_map.adaptive_threshold_c_ = 5;
  // opts_depth_map.median_filter_size_ = 5;
  // cv::Mat depth_map, confidence_map, semidense_mask;
  // mapper.getDepthMapFromDSI(depth_map, confidence_map, semidense_mask, opts_depth_map);
  
  // // Save depth map, confidence map and semi-dense mask

  // // Save semi-dense mask as an image
  // cv::imwrite("semidense_mask.png", 255 * semidense_mask);

  // // Save confidence map as an 8-bit image
  // cv::Mat confidence_map_255;
  // cv::normalize(confidence_map, confidence_map_255, 0, 255.0, cv::NORM_MINMAX, CV_32FC1);
  // cv::imwrite("confidence_map.png", confidence_map_255);

  // // Normalize depth map using given min and max depth values
  // cv::Mat depth_map_255 = (depth_map - dsi_shape.min_depth_) * (255.0 / (dsi_shape.max_depth_ - dsi_shape.min_depth_));
  // cv::imwrite("depth_map.png", depth_map_255);

  // // Save pseudo-colored depth map on white canvas
  // cv::Mat depthmap_8bit, depthmap_color;
  // depth_map_255.convertTo(depthmap_8bit, CV_8U);
  // cv::applyColorMap(depthmap_8bit, depthmap_color, cv::COLORMAP_RAINBOW);
  // cv::Mat depth_on_canvas = cv::Mat(depth_map.rows, depth_map.cols, CV_8UC3, cv::Scalar(1,1,1)*255);
  // depthmap_color.copyTo(depth_on_canvas, semidense_mask);
  // cv::imwrite("depth_colored.png", depth_on_canvas);


  // // 3. Convert semi-dense depth map to point cloud
  // EMVS::OptionsPointCloud opts_pc;
  // opts_pc.radius_search_ = 0.05;
  // opts_pc.min_num_neighbors_ = 3;
  
  // EMVS::PointCloud::Ptr pc (new EMVS::PointCloud);
  // mapper.getPointcloud(depth_map, semidense_mask, opts_pc, pc);
  
  // // Save point cloud to disk
  // pcl::io::savePCDFileASCII ("pointcloud.pcd", *pc);
  // std::cout << "Saved " << pc->points.size () << " data points to pointcloud.pcd";
  
  return 0;
}
