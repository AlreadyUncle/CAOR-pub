/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Taehun Lim (Darby) */

#include "turtlebot3_gazebo/turtlebot3_drive.h"
using namespace std;

std::string gt_txt_name;
std::string laserscan_txt_name;
//float last_timestamp = 0.0;

Turtlebot3Drive::Turtlebot3Drive()
  : nh_priv_("~")
{
  //Init gazebo ros turtlebot3 node
  ROS_INFO("TurtleBot3 Simulation Node Init");
  ROS_ASSERT(init());
}

Turtlebot3Drive::~Turtlebot3Drive()
{
  updatecommandVelocity(0.0, 0.0);
  ros::shutdown();
}

/*******************************************************************************
* Init function
*******************************************************************************/
bool Turtlebot3Drive::init()
{
  // initialize ROS parameter
  std::string cmd_vel_topic_name = nh_.param<std::string>("cmd_vel_topic_name", "");
  gt_txt_name = nh_.param<std::string>("gt_txt_name", "/home/shiqing/kitti2bag-master/kitti2bag/turtlebot3/default_gt.txt");
  laserscan_txt_name = nh_.param<std::string>("laserscan_txt_name", "/home/shiqing/kitti2bag-master/kitti2bag/turtlebot3/default_laserscan.txt");

  // initialize variables
  escape_range_       = 30.0 * DEG2RAD;
  check_forward_dist_ = 0.7;
  check_side_dist_    = 0.6;

  tb3_pose_ = 0.0;
  prev_tb3_pose_ = 0.0;

  // initialize publishers
  cmd_vel_pub_   = nh_.advertise<geometry_msgs::Twist>(cmd_vel_topic_name, 10);

  // initialize subscribers
  laser_scan_sub_  = nh_.subscribe("scan", 10, &Turtlebot3Drive::laserScanMsgCallBack, this);
  odom_sub_ = nh_.subscribe("odom", 10, &Turtlebot3Drive::odomMsgCallBack, this);

  return true;
}

void Turtlebot3Drive::odomMsgCallBack(const nav_msgs::Odometry::ConstPtr &msg)
{
  double siny = 2.0 * (msg->pose.pose.orientation.w * msg->pose.pose.orientation.z + msg->pose.pose.orientation.x * msg->pose.pose.orientation.y);
	double cosy = 1.0 - 2.0 * (msg->pose.pose.orientation.y * msg->pose.pose.orientation.y + msg->pose.pose.orientation.z * msg->pose.pose.orientation.z);  

	tb3_pose_ = atan2(siny, cosy);

  

  ofstream myfile;
  myfile.open (gt_txt_name,ios_base::app);
  float timestamp = ros::Time::now().toSec();
  float x = msg->pose.pose.position.x;
  float y = msg->pose.pose.position.y;
  float z = msg->pose.pose.position.z;
  float x_deg = msg->pose.pose.orientation.x;
  float y_deg = msg->pose.pose.orientation.y;
  float z_deg = msg->pose.pose.orientation.z;
  float w_deg = msg->pose.pose.orientation.w;
  myfile << timestamp << " " << x << " " << y << " " << z << " " << x_deg << " " << y_deg << " " << z_deg << " " << w_deg<<"\n";
  //myfile << " " << x << " " << y << " " << z << " " << x_deg << " " << y_deg << " " << z_deg << " " << w_deg<<"\n";
  myfile.close();




}

void Turtlebot3Drive::laserScanMsgCallBack(const sensor_msgs::LaserScan::ConstPtr &msg)
{
  uint16_t scan_angle[3] = {0, 30, 330};

  for (int num = 0; num < 3; num++)
  {
    if (std::isinf(msg->ranges.at(scan_angle[num])))
    {
      scan_data_[num] = msg->range_max;
    }
    else
    {
      scan_data_[num] = msg->ranges.at(scan_angle[num]);
    }
  }

  ofstream myfile;
  myfile.open (laserscan_txt_name,ios_base::app);
  float timestamp = msg->header.stamp.toSec();
  float angle_min = msg->angle_min;
  float angle_max = msg->angle_max;
  float angle_increment = msg->angle_increment;
  float time_increment =msg->time_increment;
  float scan_time = msg->scan_time;
  float range_min = msg->range_min;
  float range_max = msg->range_max;
  int size = msg->ranges.size();
  myfile << timestamp << " " << angle_min << " " << angle_max << " " << angle_increment << " " << range_min << " " << range_max; 
  for (int i =0; i<size; i++){
    myfile << " " << msg->ranges[i];
  }
  myfile << "\n";
  myfile.close();


}

void Turtlebot3Drive::updatecommandVelocity(double linear, double angular)
{
  geometry_msgs::Twist cmd_vel;

  cmd_vel.linear.x  = linear;
  cmd_vel.angular.z = angular;

  cmd_vel_pub_.publish(cmd_vel);
}

/*******************************************************************************
* Control Loop function
*******************************************************************************/
bool Turtlebot3Drive::controlLoop()
{
  static uint8_t turtlebot3_state_num = 0;

  switch(turtlebot3_state_num)
  {
    case GET_TB3_DIRECTION:
      if (scan_data_[CENTER] > check_forward_dist_)
      {
        if (scan_data_[LEFT] < check_side_dist_)
        {
          prev_tb3_pose_ = tb3_pose_;
          turtlebot3_state_num = TB3_RIGHT_TURN;
        }
        else if (scan_data_[RIGHT] < check_side_dist_)
        {
          prev_tb3_pose_ = tb3_pose_;
          turtlebot3_state_num = TB3_LEFT_TURN;
        }
        else
        {
          turtlebot3_state_num = TB3_DRIVE_FORWARD;
        }
      }

      if (scan_data_[CENTER] < check_forward_dist_)
      {
        prev_tb3_pose_ = tb3_pose_;
        turtlebot3_state_num = TB3_RIGHT_TURN;
      }
      break;

    case TB3_DRIVE_FORWARD:
      updatecommandVelocity(LINEAR_VELOCITY, 0.0);
      turtlebot3_state_num = GET_TB3_DIRECTION;
      break;

    case TB3_RIGHT_TURN:
      if (fabs(prev_tb3_pose_ - tb3_pose_) >= escape_range_)
        turtlebot3_state_num = GET_TB3_DIRECTION;
      else
        updatecommandVelocity(0.0, -1 * ANGULAR_VELOCITY);
      break;

    case TB3_LEFT_TURN:
      if (fabs(prev_tb3_pose_ - tb3_pose_) >= escape_range_)
        turtlebot3_state_num = GET_TB3_DIRECTION;
      else
        updatecommandVelocity(0.0, ANGULAR_VELOCITY);
      break;

    default:
      turtlebot3_state_num = GET_TB3_DIRECTION;
      break;
  }

  return true;
}

/*******************************************************************************
* Main function
*******************************************************************************/
int main(int argc, char* argv[])
{
  ros::init(argc, argv, "turtlebot3_drive");
  Turtlebot3Drive turtlebot3_drive;

  ros::Rate loop_rate(125);

  while (ros::ok())
  {
    turtlebot3_drive.controlLoop();
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
