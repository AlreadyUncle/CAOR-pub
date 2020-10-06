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

#include "turtlebot3_listener/turtlebot3_listener.h"
using namespace std;

std::string gt_txt_name;
std::string laserscan_txt_name;
std::string prefix;

Turtlebot3Drive::Turtlebot3Drive()
  : nh_priv_("~")
{
  //Init gazebo ros turtlebot3 node
  ROS_INFO("TurtleBot3 Listener Node Init");
  ROS_ASSERT(init());
}

Turtlebot3Drive::~Turtlebot3Drive()
{
  ros::shutdown();
}

/*******************************************************************************
* Init function
*******************************************************************************/
bool Turtlebot3Drive::init()
{
  // initialize ROS parameter
  gt_txt_name = nh_.param<std::string>("gt_txt_name", "/home/shiqing/kitti2bag-master/kitti2bag/turtlebot3/default_gt.txt");
  laserscan_txt_name = nh_.param<std::string>("laserscan_txt_name", "/home/shiqing/kitti2bag-master/kitti2bag/turtlebot3/default_laserscan.txt");
  prefix = nh_.param<std::string>("prefix", "");
  // initialize subscribers
  laser_scan_sub_  = nh_.subscribe(prefix + "scan", 10, &Turtlebot3Drive::laserScanMsgCallBack, this);
  odom_sub_ = nh_.subscribe(prefix + "odom", 10, &Turtlebot3Drive::odomMsgCallBack, this);

  return true;
}

void Turtlebot3Drive::odomMsgCallBack(const nav_msgs::Odometry::ConstPtr &msg)
{
  
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
    // turtlebot3_drive.controlLoop();
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
