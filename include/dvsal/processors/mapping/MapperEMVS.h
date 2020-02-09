//---------------------------------------------------------------------------------------------------------------------
//  EMVS https://github.com/uzh-rpg/rpg_emvs
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2018
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

#ifndef DVSAL_MAPPING_MAPPER_EMVS_H_
#define DVSAL_MAPPING_MAPPER_EMVS_H_

#include <dvsal/processors/mapping/PinholeCamera.h>
#include <dvsal/processors/mapping/Trajectory.h>
#include <dvsal/processors/mapping/DepthVector.h>
#include <dvsal/processors/mapping/Cartesian3DGrid.h>

#include <dv-sdk/processing.hpp>
#include <dv-sdk/config.hpp>
#include <dv-sdk/utils.h>

#include <pcl/point_types.h>
#include <pcl/point_cloud.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv/cv.hpp>

#include <Eigen/Eigen>

namespace EMVS {

  typedef pcl::PointXYZI PointType;
  typedef pcl::PointCloud<PointType> PointCloud;

  #ifdef USE_INVERSE_DEPTH
  using TypeDepthVector = InverseDepthVector;
  #else
  using TypeDepthVector = LinearDepthVector;
  #endif

  struct ShapeDSI {
  public:

    ShapeDSI(){}
    
    ShapeDSI(size_t dimX, size_t dimY, size_t dimZ, float min_depth, float max_depth, float fov){
      dimX_ = dimX;
      dimY_ = dimY;
      dimZ_ = dimZ;
      min_depth_ = min_depth;
      max_depth_ = max_depth;
      fov_ = fov;   
    }

    size_t dimX_;
    size_t dimY_;
    size_t dimZ_;

    float min_depth_;
    float max_depth_;

    // Field of View
    float fov_;
  };


  struct OptionsDepthMap{
    // Adaptive Gaussian Thresholding parameters
    int adaptive_threshold_kernel_size_;
    double adaptive_threshold_c_;
    
    // Kernel size of median filter
    int median_filter_size_;
  };


  struct OptionsPointCloud{
    // Outlier removal parameters
    float radius_search_;
    int min_num_neighbors_;
  };


  typedef LinearTrajectory TrajectoryType;

  class MapperEMVS{
  public:

    MapperEMVS(){}
    
    MapperEMVS(const std::vector<float>& _camParams, const ShapeDSI& _dsiShape);

    bool evaluateDSI(const std::vector<dv::Event>& events,
                     const TrajectoryType& trajectory,
                     const Eigen::Matrix4f& T_rv_w);
    
    void getDepthMapFromDSI(cv::Mat& depth_map, cv::Mat &confidence_map, cv::Mat &mask, const OptionsDepthMap &options_depth_map);

    void getPointcloud(const cv::Mat& depth_map,
                       const cv::Mat& mask,
                       const OptionsPointCloud &options_pc,
                       PointCloud::Ptr &pc_);
    
    Grid3D dsi_;

  private:

    void setupDSI();

    void precomputeRectifiedPoints();

    void fillVoxelGrid(const std::vector<Eigen::Vector4f> &event_locations_z0,
                      const std::vector<Eigen::Vector3f> &camera_centers);
    
    void convertDepthIndicesToValues(const cv::Mat &depth_cell_indices, cv::Mat &depth_map);

    void removeMaskBoundary(cv::Mat& mask, int border_size);
    

    // Intrinsics of the camera
    Eigen::Matrix3f K_;
    int width_;
    int height_;

    // (Constant) parameters that define the DSI (size and intrinsics)
    ShapeDSI dsiShape_;
    dvsal::PinholeCamera virtualCam_;

    // Precomputed vector of num_depth_cells_ inverse depths,
    // uniformly sampled in inverse depth space
    TypeDepthVector depths_vec_;
    std::vector<float> raw_depths_vec_;

    // Precomputed (normalized) bearing vectors for each pixel of the reference image
    Eigen::Matrix2Xf precomputed_rectified_points_;

    const size_t packet_size_ = 1024;

  };

}

#endif