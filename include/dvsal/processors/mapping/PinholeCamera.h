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

#ifndef DVSAL_MAPPING_GEOMETRY_UTILS_H_
#define DVSAL_MAPPING_GEOMETRY_UTILS_H_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv/cv.hpp>

#include <Eigen/Eigen>

namespace dvsal {

  typedef float Depth;
  typedef float InverseDepth;

  class PinholeCamera{
  public:
    PinholeCamera() {}
    PinholeCamera(int width, int height, float fx, float fy, float cx, float cy){
      width_ = width;
      height_ = height;
      fx_ = fx; fy_ = fy;
      cx_ = cx; cy_ = cy;

      K_ << (float) fx_, 0.f          , (float) cx_,
            0.f        , (float) fy_  , (float) cy_,
            0.f        , 0.f          ,  1.f;

      cvK_ = cv::Mat(K_.rows(), K_.cols(), CV_32FC1, K_.data());

      Kinv_ = K_.inverse();
    }

    inline Eigen::Vector2d project3dToPixel(const Eigen::Vector3d& P){
      // CHECK_GE(std::fabs(P[2]), 1e-6);
      return Eigen::Vector2d(fx_*P[0]/P[2] + cx_, fy_*P[1]/P[2] + cy_);
    }

    inline Eigen::Vector3d projectPixelTo3dRay(const Eigen::Vector2d & u){
      return Eigen::Vector3d((u[0]-cx_)/fx_, (u[1]-cy_)/fy_, 1.0);
    }

    inline Eigen::Vector2f rectifyPoint(Eigen::Vector2f _distorPoint){
      // 666
      std::vector<cv::Point2f> undistortedCvPoint;
      std::vector<cv::Point2f> cvPoints = {{_distorPoint[0],_distorPoint[1]}};
      cv::undistortPoints(cvPoints, undistortedCvPoint,cvK_, D_);

      Eigen::Vector2f undistortedPoint(undistortedCvPoint[0].x , undistortedCvPoint[0].y);
      return undistortedPoint;
    }

    inline Eigen::Vector2f rectifyPoint(Eigen::Vector2f _distorPoint, cv::Mat _K , cv::Mat _D){
      // 666
      std::vector<cv::Point2f> undistortedCvPoint;
      std::vector<cv::Point2f> cvPoints = {{_distorPoint[0],_distorPoint[1]}};
      cv::undistortPoints(cvPoints, undistortedCvPoint,_K, _D);

      Eigen::Vector2f undistortedPoint(undistortedCvPoint[0].x , undistortedCvPoint[0].y);
      return undistortedPoint;
    }

    Eigen::Matrix3f K() const{
      return K_;
    }

    int width_;
    int height_;
    float fx_;
    float fy_;
    float cx_;
    float cy_;
    Eigen::Matrix3f K_;
    Eigen::Matrix3f Kinv_;
    cv::Mat cvK_;
    cv::Mat D_ = (cv::Mat_<float>(5,1) << -0.1385927674081299, 0.09337366641919795, -0.0003355869875320301, 0.0001737201582276446, 0.0);
  };

}


#endif