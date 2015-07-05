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
#include <sensor_msgs/CameraInfo.h>
//#include <sensor_msgs/PointCloud2.h>
//#include <tf/transform_listener.h>
#include <cob_perception_msgs/DetectionArray.h>

// topics
#include <message_filters/subscriber.h>
#include <message_filters/synchronizer.h>
#include <message_filters/sync_policies/approximate_time.h>
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
#include "ftfootb_label_reading/TextTagLocalization.h"
#include "ros/package.h"

class LabelReader
{

protected:

	typedef message_filters::sync_policies::ApproximateTime<sensor_msgs::Image, sensor_msgs::CameraInfo> ColorImageSyncPolicy;

//	ros::Subscriber point_cloud_sub_;
//	ros::Publisher point_cloud_pub_; ///< Point cloud output topic
	ros::Publisher detections_pub_;		///< Publishes the detected tags
	//message_filters::Subscriber<sensor_msgs::PointCloud2> point_cloud_sub_;	///< Point cloud input topic
	image_transport::ImageTransport* it_;
	image_transport::SubscriberFilter color_camera_image_sub_;	///< Color camera image input topic
	message_filters::Subscriber<sensor_msgs::CameraInfo> color_camera_info_sub_;	///< camera information service
	boost::shared_ptr<message_filters::Synchronizer<ColorImageSyncPolicy > > color_image_sub_sync_; ///< Synchronizer
	//image_transport::Publisher color_camera_image_pub_;		///< Color camera image output topic
	//message_filters::Synchronizer<message_filters::sync_policies::ApproximateTime<sensor_msgs::PointCloud2, sensor_msgs::Image> >* sync_pointcloud_;	///< synchronizer for input data
	//message_filters::Connection sync_pointcloud_callback_connection_;

	//tf::TransformListener* transform_listener_;

    cv::Mat camera_matrix_;
    bool camera_matrix_initialized_;

	ros::NodeHandle node_handle_; ///< ROS node handle

	double roi_height_;		///<
	std::string path_;	///< path to the package of this software
	std::string path_data_;		///< path to the data files

	MatchTemplate match_template_;
	TextTagDetection text_tag_detection_;
	FeatureRepresentation feature_representation_;
	TextTagLocalization text_tag_localization_;

	// parameters
	double metric_tag_width_;		///< metric tag dimensions (in [m])
	double metric_tag_height_;		///< metric tag dimensions (in [m])
	bool display_results_;			///< display the detections in a live image on/off
	double text_tag_detection_sensitivity_;		///< sensitivity of text tag detection, the lower the number, the more detections (and false detections) may be obtained (value range [0, 1])
	int tag_detection_target_image_width_;		///< target image width used for tag detection in the image with Viola-Jones classifier
	int tag_detection_target_image_height_;		///< target image height used for tag detection in the image with Viola-Jones classifier
	int training_data_source_;	///< 0 - load all letter or number symbols from one file and generate further training samples;  1 - load training data from raw images;  2 - load training data from yml file
	int classifier_;		///< 1-KNN, 2-train svm, 3-load svm
	int feature_number_;		///< feature number: 1. HOG 2. LBP 3. BRIEF
	int single_or_combination_;	///< single or combinations (pairs) of letters/numbers: 1. single 2. combinations
	int recognition_method_;		///< template matching OR feature representation: 1. TM, 2. FR, 3. both.
	int template_matching_method_;	///< template matching method: 0: SQDIFF \n 1: SQDIFF NORMED \n 2: CCORR \n 3: CCORR NORMED \n 4: COEFF \n 5: COEFF NORMED.

public:
	LabelReader(ros::NodeHandle nh);

	~LabelReader();

//	unsigned long init();

	unsigned long convertImageMessageToMat(const sensor_msgs::Image::ConstPtr& color_image_msg, cv_bridge::CvImageConstPtr& color_image_ptr, cv::Mat& color_image);

//	template <typename T>
//	void inputCallback(const sensor_msgs::PointCloud2::ConstPtr& point_cloud_msg);

	void imageCallback(const sensor_msgs::ImageConstPtr& color_camera_data, const sensor_msgs::CameraInfoConstPtr& color_camera_info);

//	void imgConnectCB(const image_transport::SingleSubscriberPublisher& pub);
//
//	void imgDisconnectCB(const image_transport::SingleSubscriberPublisher& pub);
//
//	void pcConnectCB(const ros::SingleSubscriberPublisher& pub);
//
//	void pcDisconnectCB(const ros::SingleSubscriberPublisher& pub);

	void setParams(ros::NodeHandle & nh);
};
