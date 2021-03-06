/****************************************************************
 *
 * Copyright (c) 2010
 *
 * Fraunhofer Institute for Manufacturing Engineering
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: care-o-bot
 * ROS stack name: ftfootb
 * ROS package name: ftfootb_label_reading
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Author:  Wenzhe Li, email:liwenzhe0914@gmail.com
 * 			Richard Bormann, email:richard.bormann@ipa.fhg.de
 * Supervised by: Richard Bormann, email:richard.bormann@ipa.fhg.de
 *
 * Date of creation: August 2014
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Fraunhofer Institute for Manufacturing
 *       Engineering and Automation (IPA) nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/

//##################
//#### includes ####

// ROS includes
#include <ros/ros.h>

// ROS message includes
#include <sensor_msgs/Image.h>
//#include <sensor_msgs/PointCloud2.h>
//#include <tf/transform_listener.h>

// topics
//#include <message_filters/subscriber.h>
//#include <message_filters/synchronizer.h>
//#include <message_filters/sync_policies/approximate_time.h>
#include <image_transport/image_transport.h>
#include <image_transport/subscriber_filter.h>

// opencv
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>

// point cloud
//#include <pcl/point_types.h>
//#include <pcl_ros/point_cloud.h>

// System
#include <iostream>

// project
#include "ftfootb_label_reading/MatchTemplate.h"
#include "ftfootb_label_reading/FeatureRepresentation.h"
#include "ftfootb_label_reading/TextTagDetection.h"
#include "ros/package.h"

class LabelReader
{
protected:
//	ros::Subscriber point_cloud_sub_;
//	ros::Publisher point_cloud_pub_; ///< Point cloud output topic
	//message_filters::Subscriber<sensor_msgs::PointCloud2> point_cloud_sub_;	///< Point cloud input topic
	image_transport::ImageTransport* it_;
	image_transport::SubscriberFilter color_camera_image_sub_;	///< Color camera image input topic
	//image_transport::Publisher color_camera_image_pub_;		///< Color camera image output topic
	//message_filters::Synchronizer<message_filters::sync_policies::ApproximateTime<sensor_msgs::PointCloud2, sensor_msgs::Image> >* sync_pointcloud_;	///< synchronizer for input data
	//message_filters::Connection sync_pointcloud_callback_connection_;

	//tf::TransformListener* transform_listener_;



	double roi_height_;

	ros::NodeHandle node_handle_; ///< ROS node handle

public:

	MatchTemplate match_template_;
	TextTagDetection text_tag_detection_;
	FeatureReprenstation feature_reprenstation_;

	LabelReader(ros::NodeHandle nh);

	~LabelReader();

	unsigned long init();

	unsigned long convertImageMessageToMat(const sensor_msgs::Image::ConstPtr& color_image_msg, cv_bridge::CvImageConstPtr& color_image_ptr, cv::Mat& color_image);

//	template <typename T>
//	void inputCallback(const sensor_msgs::PointCloud2::ConstPtr& point_cloud_msg);

	void imageCallback(const sensor_msgs::ImageConstPtr& color_image_msg);

	void imgConnectCB(const image_transport::SingleSubscriberPublisher& pub);

	void imgDisconnectCB(const image_transport::SingleSubscriberPublisher& pub);

	void pcConnectCB(const ros::SingleSubscriberPublisher& pub);

	void pcDisconnectCB(const ros::SingleSubscriberPublisher& pub);

	void setParams(ros::NodeHandle & nh);

private:

	int load, classifier,feature_number,single_or_combination,recognition_method,template_matching_method;
};
