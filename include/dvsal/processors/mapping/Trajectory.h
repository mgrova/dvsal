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

#ifndef DVSAL_MAPPING_TRAJECTORY_H_
#define DVSAL_MAPPING_TRAJECTORY_H_

#include <map>
#include <iostream>

template<class DerivedTrajectory>
class Trajectory
{
public:  
  typedef std::map<float, Eigen::Matrix4f> PoseMap;
  
  DerivedTrajectory& derived()
  { return static_cast<DerivedTrajectory&>(*this); }

  Trajectory() {}

  Trajectory(const PoseMap& poses)
    : poses_(poses)
  {}

  // Returns T_W_C (mapping points from C to the world frame W)
  bool getPoseAt(const float& t, Eigen::Matrix4f& T) const
  {
    return derived().getPoseAt(t, T);
  }
    
  void getFirstControlPose(Eigen::Matrix4f* T, float* t) const
  {
    *t = poses_.begin()->first;
    *T = poses_.begin()->second;
  }

  void getLastControlPose(Eigen::Matrix4f* T, float* t) const
  {
    *t = poses_.rbegin()->first;
    *T = poses_.rbegin()->second;
  }
  
  size_t getNumControlPoses() const
  {
    return poses_.size();
  }
    
  bool print() const
  {
    size_t control_pose_idx = 0u;
    for(auto it : poses_)
    {
      std::cout << "--Control pose #" << control_pose_idx++ << ". time = " << it.first;
      std::cout << "--T = ";
      std::cout << it.second;
    }
    return true;
  }

protected:
  PoseMap poses_;

};



class LinearTrajectory : public Trajectory<LinearTrajectory>
{
public:

  LinearTrajectory() : Trajectory() {}

  LinearTrajectory(const PoseMap& poses)
    : Trajectory(poses)
  {
    if (poses_.size() < 2)
      std::cout << "At least two poses need to be provided \n";
  }

  bool getPoseAt(const float& t, Eigen::Matrix4f& T) const
  {
    float t0_, t1_;
    Eigen::Matrix4f T0_ = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f T1_ = Eigen::Matrix4f::Identity();

    // Check if it is between two known poses
    auto it1 = poses_.upper_bound(t);
    if(it1 ==  poses_.begin())
    {
      std::cout << "Cannot extrapolate in the past. Requested pose: "
                              << t << " but the earliest pose available is at time: "
                              << poses_.begin()->first << "\n";
      return false;
    }
    else if(it1 == poses_.end())
    {
      std::cout << "Cannot extrapolate in the future. Requested pose: "
                              << t << " but the latest pose available is at time: "
                              << (poses_.rbegin())->first << "\n";
      return false;
    }
    else
    {
      auto it0 = std::prev(it1);
      t0_ = (it0)->first;
      T0_ = (it0)->second;
      t1_ = (it1)->first;
      T1_ = (it1)->second;
    }

    // 666 MUST implement Linear interpolation in SE(3)
    Eigen::Matrix4f T_relative = Eigen::Matrix4f::Identity();
    T_relative = T0_.inverse() * T1_;
    auto delta_t = (t - t0_) / (t1_ - t0_);

    // T = T0_ * Eigen::Matrix4f::exp(delta_t * T_relative.log());

    std::cout << "T" << T1_ << "\n";

    T = T_relative;
    return true;
  }

};

#endif