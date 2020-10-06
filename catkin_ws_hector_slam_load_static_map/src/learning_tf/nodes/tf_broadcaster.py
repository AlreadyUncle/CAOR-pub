#!/usr/bin/env python  
import roslib
roslib.load_manifest('learning_tf')
import rospy
from geometry_msgs.msg import PoseWithCovarianceStamped
import tf
from tf.transformations import quaternion_from_euler


def talker():
    pub = rospy.Publisher('initialpose', PoseWithCovarianceStamped, queue_size=1)
    rate = rospy.Rate(10) # 10hz
    while not rospy.is_shutdown():
        ps_cov = PoseWithCovarianceStamped()
        ps_cov.header.frame_id = "map"
        ps_cov.header.stamp = rospy.get_rostime()
        ps_cov.pose.pose.position.x = x
        ps_cov.pose.pose.position.y = y
        ps_cov.pose.pose.position.z = z
        ori = quaternion_from_euler(roll, pitch, yaw)
        ps_cov.pose.pose.orientation.x = ori[0]
        ps_cov.pose.pose.orientation.y = ori[1]
        ps_cov.pose.pose.orientation.z = ori[2]
        ps_cov.pose.pose.orientation.w = ori[3]
        ps_cov.pose.covariance = cov
        pub.publish(ps_cov)
        rate.sleep()


def cov_matrix(x_cov, y_cov, z_cov):
    return [x_cov, 0, 0, 0, 0, 0,
            0, y_cov, 0, 0, 0, 0,
            0, 0, z_cov, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0]


if __name__ == '__main__':
    rospy.init_node('initial_pose_broadcaster')
    cov_x = rospy.get_param("~cov_x", 0.6)
    cov_y = rospy.get_param("~cov_y", 0.6)
    cov_z = rospy.get_param("~cov_z", 0.6)
    x = rospy.get_param("~x", -512.0)
    y = rospy.get_param("~y", -512.0)
    z = rospy.get_param("~z", 0.0)
    roll = rospy.get_param("~roll", 0.0)
    pitch = rospy.get_param("~pitch", 0.0)
    yaw = rospy.get_param("~yaw", 0.0)
    cov = cov_matrix(cov_x, cov_y, cov_z)


    try:
        talker()
    except rospy.ROSInterruptException:
        pass