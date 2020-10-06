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

#include "turtlebot3_autodrive/turtlebot3_autodrive.h"
using namespace std;


std::string path_filename;
std::string prefix;
int point_num = 0;
vector<float> vecx;
vector<float> vecy;
float x;
float y;
int i = 0;

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
  prefix = nh_.param<std::string>("prefix", "");
  path_filename = nh_.param<std::string>("path_filename", "");
  ifstream inFile;
  inFile.open(path_filename);
  string line;

  while(getline(inFile, line)){
    float value;
    stringstream ss(line);

    ss>>value;
    vecx.push_back(value);
    ss>>value;
    vecy.push_back(value);
    
    point_num++;

  }
  inFile.close();


  // initialize variables
  check_dist = 0.1;
  check_angle = 5.0 * DEG2RAD;

  tb3_pose_ = 0.0;
  prev_tb3_pose_ = 0.0;

  // initialize publishers
  cmd_vel_pub_   = nh_.advertise<geometry_msgs::Twist>(prefix + cmd_vel_topic_name, 10);

  // initialize subscribers
  odom_sub_ = nh_.subscribe(prefix + "odom", 10, &Turtlebot3Drive::odomMsgCallBack, this);

  return true;
}

void Turtlebot3Drive::odomMsgCallBack(const nav_msgs::Odometry::ConstPtr &msg)
{
  double siny = 2.0 * (msg->pose.pose.orientation.w * msg->pose.pose.orientation.z + msg->pose.pose.orientation.x * msg->pose.pose.orientation.y);
	double cosy = 1.0 - 2.0 * (msg->pose.pose.orientation.y * msg->pose.pose.orientation.y + msg->pose.pose.orientation.z * msg->pose.pose.orientation.z);  
  x = msg->pose.pose.position.x;
  y = msg->pose.pose.position.y;
	tb3_pose_ = atan2(siny, cosy);
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
{ float delta_x = vecx[i] - x;
  float delta_y = vecy[i] - y;
  float delta_theta = atan2(delta_y,delta_x) - tb3_pose_;
  if (fabs(delta_theta) > check_angle){
    if (fabs(delta_theta) <= M_PI){
      if (delta_theta>=0){
        updatecommandVelocity(0, ANGULAR_VELOCITY);
      }
      else{
        updatecommandVelocity(0, -ANGULAR_VELOCITY);
      }
    }
    else{
      if (delta_theta>=0){
        updatecommandVelocity(0, -ANGULAR_VELOCITY);
      }
      else{
        updatecommandVelocity(0, ANGULAR_VELOCITY);

      }
    }
    return true;
  }

  if(delta_x * delta_x + delta_y * delta_y > check_dist * check_dist){
    updatecommandVelocity(LINEAR_VELOCITY , 0);
  }
  else{
    updatecommandVelocity(0, 0);
    i++; 
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
  { if (i == point_num){
    break;
    }
    turtlebot3_drive.controlLoop();
    ros::spinOnce();
    loop_rate.sleep();
  }

  return 0;
}
