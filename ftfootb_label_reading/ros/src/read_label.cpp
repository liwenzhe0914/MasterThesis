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

#define MAIN

//##################
//#### includes ####

#include <ftfootb_label_reading/read_label.h>
#include <ftfootb_label_reading/timer.h>

void LabelReader::setParams(ros::NodeHandle& nh)
{
	node_handle_.param("metric_tag_width", metric_tag_width_, 0.391);
	std::cout << "metric_tag_width = " << metric_tag_width_ << "\n";
	node_handle_.param("metric_tag_height", metric_tag_height_, 0.065);
	std::cout << "metric_tag_height = " << metric_tag_height_ << "\n";
	node_handle_.param("display_results", display_results_, false);
	std::cout << "display_results = " << display_results_ << "\n";
	node_handle_.param("tag_detection_target_image_width", tag_detection_target_image_width_, -1);
	std::cout << "tag_detection_target_image_width = " << tag_detection_target_image_width_ << "\n";
	node_handle_.param("tag_detection_target_image_height", tag_detection_target_image_height_, -1);
	std::cout << "tag_detection_target_image_height = " << tag_detection_target_image_height_ << "\n";
	node_handle_.param("load", load_, 0);
	std::cout << "load = " << load_ << "\n";
	node_handle_.param("classifier", classifier_, 1);
	std::cout << "classifier = " << classifier_ << "\n";
	node_handle_.param("feature_number", feature_number_, 1);
	std::cout << "feature_number = " << feature_number_ << "\n";
	node_handle_.param("single_or_combination", single_or_combination_, 1);
	std::cout << "single_or_combination = " << single_or_combination_ << "\n";
	node_handle_.param("recognition_method", recognition_method_, 2);
	std::cout << "recognition_method = " << recognition_method_ << "\n";
	node_handle_.param("template_matching_method", template_matching_method_, 5);
	std::cout << "template_matching_method = " << template_matching_method_ << "\n";
}

LabelReader::LabelReader(ros::NodeHandle nh)
: 	path_(ros::package::getPath("ftfootb_label_reading") + "/"), path_data_ (path_ + "common/files/"), match_template_(path_data_),
  	text_tag_detection_(path_data_, 0., 0.), node_handle_(nh)
{
	setParams(node_handle_);

	text_tag_detection_.set_tag_properties(metric_tag_width_, metric_tag_height_);

	camera_matrix_initialized_ = false;

	if (classifier_==2 || classifier_==3) // SVM for classification
	{
		feature_representation_.load_or_train_SVM_classifiers(feature_representation_.numbers_svm,feature_representation_.letters_svm,
															load_,classifier_,feature_number_,single_or_combination_,path_data_);
	}
	else if (classifier_==1) // KNN for classification
	{
		feature_representation_.load_or_train_KNN_classifiers(feature_representation_.numbers_knn,feature_representation_.letters_knn,
															load_,classifier_,feature_number_,single_or_combination_,path_data_);
	}
	else
		std::cout<<"[ROS Read label ERROR] wrong *classifier_* parameter given! "<<std::endl;

	// subscribe image topics
	it_ = new image_transport::ImageTransport(node_handle_);
	//color_camera_image_sub_.registerCallback(boost::bind(&LabelReader::imageCallback, this, _1));
	color_camera_image_sub_.subscribe(*it_, "image_color", 1);
	color_camera_info_sub_.subscribe(node_handle_, "camera_info", 1);

    color_image_sub_sync_ = boost::shared_ptr<message_filters::Synchronizer<ColorImageSyncPolicy> >(new message_filters::Synchronizer<ColorImageSyncPolicy>(ColorImageSyncPolicy(3)));
    color_image_sub_sync_->connectInput(color_camera_image_sub_, color_camera_info_sub_);
    color_image_sub_sync_->registerCallback(boost::bind(&LabelReader::imageCallback, this, _1, _2));

    // publishers
    detections_pub_ = node_handle_.advertise<cob_perception_msgs::DetectionArray>("text_tag_detections", 1);

	std::cout << "LabelReader initialized." << std::endl;
}

LabelReader::~LabelReader()
{
	if (it_ != 0)
		delete it_;
//		if (sync_pointcloud_ != 0)
//			delete sync_pointcloud_;
//	if (transform_listener_ != 0)
//		delete transform_listener_;
}

//unsigned long LabelReader::init()
//{
//	//color_camera_image_sub_.subscribe(*it_, "colorimage", 1);
//	//point_cloud_sub_.subscribe(node_handle_, "pointcloud", 1);
//
//	//sync_pointcloud_->connectInput(point_cloud_sub_, color_camera_image_sub_);
//	//sync_pointcloud_callback_connection_ = sync_pointcloud_->registerCallback(boost::bind(&CobKinectImageFlipNodelet::inputCallback, this, _1, _2));
//
//	return 0;
//}

unsigned long LabelReader::convertImageMessageToMat(const sensor_msgs::Image::ConstPtr& image_msg, cv_bridge::CvImageConstPtr& image_ptr, cv::Mat& image)
{
	try
	{
		image_ptr = cv_bridge::toCvShare(image_msg, image_msg->encoding);
	}
	catch (cv_bridge::Exception& e)
	{
		ROS_ERROR("text tag reading: cv_bridge exception: %s", e.what());
		return 1;
	}

	image = image_ptr->image;

	return 0;
}

void LabelReader::imageCallback(const sensor_msgs::ImageConstPtr& color_camera_data, const sensor_msgs::CameraInfoConstPtr& color_camera_info)
{
	// set camera matrix on first call
	if (camera_matrix_initialized_ == false)
	{
		camera_matrix_ = cv::Mat::zeros(3,3,CV_64FC1);
		camera_matrix_.at<double>(0,0) = color_camera_info->K[0];
		camera_matrix_.at<double>(0,2) = color_camera_info->K[2];
		camera_matrix_.at<double>(1,1) = color_camera_info->K[4];
		camera_matrix_.at<double>(1,2) = color_camera_info->K[5];
		camera_matrix_.at<double>(2,2) = 1;
		camera_matrix_initialized_ = true;
	}

	Timer tim;

	// 1. get image from message
	cv_bridge::CvImageConstPtr image_ptr;
	cv::Mat image, image_grayscale, image_grayscale_small, image_display;
	convertImageMessageToMat(color_camera_data, image_ptr, image);
	if (image.channels() == 1)
	{
		// monochrome image
		image_grayscale = image;	//.clone();
		if (display_results_)
			cvtColor(image, image_display, CV_GRAY2BGR);
	}
	else if (image.channels() == 3)
	{
		// color image
		cvtColor(image, image_grayscale, CV_BGR2GRAY);
		if (display_results_)
			image_display = image.clone();
	}
	else
	{
		ROS_ERROR("LabelReader::imageCallback: Wrong number of channels in camera image. Allowed is 1 or 3.");
		return;
	}
	if (tag_detection_target_image_width_>0 && tag_detection_target_image_height_>0)
		cv::resize(image_grayscale, image_grayscale_small, cv::Size(tag_detection_target_image_width_, tag_detection_target_image_height_));
	else
		image_grayscale_small = image_grayscale;

	std::cout << "image received" << std::endl;

	// 2. find text tags in original image
	std::vector<TagDetectionData> detection_list_r;
	text_tag_detection_.text_tag_detection_fine_detection_rectangle_detection(image_grayscale_small, detection_list_r);
	std::cout << detection_list_r.size() << " text tags detected!" << std::endl;
	std::cout << "Text Detection: [" << tim.getElapsedTimeInMilliSec() << " ms] processing time" << std::endl;

	// correct scale of detections
	if (tag_detection_target_image_width_>0 && tag_detection_target_image_height_>0)
	{
		const double factor_x = image.cols/(double)tag_detection_target_image_width_;
		const double factor_y = image.rows/(double)tag_detection_target_image_height_;
//		for (size_t i=0; i<detection_list.size(); ++i)
//		{
//			detection_list[i].x *= factor_x;
//			detection_list[i].width *= factor_x;
//			detection_list[i].y *= factor_y;
//			detection_list[i].height *= factor_y;
//		}
		for (size_t i=0; i<detection_list_r.size(); ++i)
		{
			detection_list_r[i].min_area_rect_.center.x *= factor_x;
			detection_list_r[i].min_area_rect_.center.y *= factor_y;
			detection_list_r[i].min_area_rect_.size.width *= factor_x;
			detection_list_r[i].min_area_rect_.size.height *= factor_y;
			for (size_t k=0; k<detection_list_r[i].corners_.size(); ++k)
			{
				detection_list_r[i].corners_[k].x *= factor_x;
				detection_list_r[i].corners_[k].y *= factor_y;
			}
		}
	}

	// 3. read texts from tags, determine 3d coordinates
	cob_perception_msgs::DetectionArray detection_array;
	std_msgs::Header header = color_camera_data->header;
	for (size_t i=0; i< detection_list_r.size(); ++i)
	{
		if (detection_list_r[i].min_area_rect_.center.x!=0 && detection_list_r[i].min_area_rect_.center.y!=0)
		{
			std::cout << "reading starts"<<std::endl;
			//cv::rectangle(image_display, detection_list[i], cv::Scalar(0,0,255), 1, 8, 0);
			if (display_results_)
				for(int j=0; j<4; ++j)
					cv::line(image_display, detection_list_r[i].corners_[j], detection_list_r[i].corners_[(j+1)%4], cv::Scalar(0,0,255), 1, 8);

			// get rectified image roi for reading from it
			cv::Mat roi;	// = image_grayscale(detection_list[i]);
			text_tag_detection_.remove_projection(detection_list_r[i], image_grayscale, roi);
			// verify with template
			double score = text_tag_detection_.compare_detection_with_template(roi);
			if (score <= 0.5)
				continue;

			std::string tag_label_features, tag_label_template_matching;
			if (recognition_method_==2 || recognition_method_==3)
			{
				if (classifier_==2 || classifier_==3) // SVM for classification
				{
					// todo: check for const image
					tag_label_features=feature_representation_.read_text_tag_SVM(feature_representation_.numbers_svm,feature_representation_.letters_svm,roi,
																				feature_number_,single_or_combination_);
				}

				else if (classifier_==1) // KNN for classification
				{
					tag_label_features=feature_representation_.read_text_tag_KNN(feature_representation_.numbers_knn,feature_representation_.letters_knn,roi,
																				feature_number_,single_or_combination_);
				}
				else
					std::cout<<"[Read label ERROR] wrong *load* parameter given! "<<std::endl;

				if (display_results_)
					//cv::putText(image_display, tag_label_features, cv::Point(detection_list[i].x, detection_list[i].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(0,0,255), 2);
					cv::putText(image_display, tag_label_features, cv::Point(detection_list_r[i].corners_[1].x, detection_list_r[i].corners_[1].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(0,0,255), 2);
			}

			if (recognition_method_==1 || recognition_method_==3)
			{
				match_template_.read_tag(roi, tag_label_template_matching,template_matching_method_);
				std::cout << "The text tag reads: " << tag_label_template_matching << "." << std::endl;
				if (display_results_)
					//cv::putText(image_display, tag_label_template_matching, cv::Point(detection_list[i].x-100, detection_list[i].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(255,0,255), 2);
					cv::putText(image_display, tag_label_template_matching, cv::Point(detection_list_r[i].corners_[1].x-100, detection_list_r[i].corners_[1].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(255,0,255), 2);
			}

			// determine 3d coordinates
			tf::Pose pose;
			text_tag_localization_.locate_tag(detection_list_r[i], pose, camera_matrix_.at<double>(0,0), camera_matrix_.at<double>(1,1), camera_matrix_.at<double>(0,2), camera_matrix_.at<double>(1,2), metric_tag_width_, metric_tag_height_);

			// store to message
			cob_perception_msgs::Detection detection;
			detection.header = header;
			detection.label = tag_label_features;
			detection.pose.header = header;
			tf::poseTFToMsg(pose, detection.pose.pose);
			detection.bounding_box_lwh.x = metric_tag_width_;
			detection.bounding_box_lwh.y = metric_tag_height_;
			detection.bounding_box_lwh.z = 0.01;
			detection_array.detections.push_back(detection);
		}
	}

	// display timing and image
	double time_in_mseconds = tim.getElapsedTimeInMilliSec();
	std::cout << "Whole system [" << time_in_mseconds << " ms] processing time" << std::endl;
	std::stringstream ss;
	ss<<time_in_mseconds<<"ms processing time";
	if (display_results_)
	{
		cv::putText(image_display, ss.str(), cv::Point(10, 30), cv::FONT_HERSHEY_PLAIN, 1.5, CV_RGB(255,0,0), 2);
		ss.str("");
		cv::imshow("image", image_display);
		cv::waitKey(1);
	}

	// publish detections
	detection_array.header = header;
	detections_pub_.publish(detection_array);

	//	// create ReadLabel object (from common folder) and call detect function
//
//	// publish image
//	cv_bridge::CvImage cv_ptr;
//	cv_ptr.image = color_image_turned;
//	cv_ptr.encoding = "bgr8";
//	sensor_msgs::Image::Ptr color_image_turned_msg = cv_ptr.toImageMsg();
//	color_image_turned_msg->header = color_image_msg->header;
//	color_camera_image_pub_.publish(color_image_turned_msg);
}

//#######################
//#### main programm ####
int main(int argc, char** argv)
{
	// Initialize ROS, specify name of node
	ros::init(argc, argv, "ftfootb_label_reading");

	// Create a handle for this node, initialize node
	ros::NodeHandle nh("~");

	// Create LabelReader class instance
	LabelReader label_reader(nh);
	ros::spin();

	return 0;
}
