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
	node_handle_.param("tag_detection_target_image_width", tag_detection_target_image_width_, 640);
	std::cout << "tag_detection_target_image_width = " << tag_detection_target_image_width_ << "\n";
	node_handle_.param("tag_detection_target_image_height", tag_detection_target_image_height_, 480);
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
: 	path_(ros::package::getPath("ftfootb_label_reading") + "/"), path_data_ (path_ + "common/files/"), match_template_(path_data_), text_tag_detection_(path_data_), node_handle_(nh)
{
	setParams(node_handle_);

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

	it_ = new image_transport::ImageTransport(node_handle_);
	color_camera_image_sub_.registerCallback(boost::bind(&LabelReader::imageCallback, this, _1));
	color_camera_image_sub_.subscribe(*it_, "colorimage_in", 1);

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

void LabelReader::imageCallback(const sensor_msgs::ImageConstPtr& image_msg)
{
	Timer tim;

	// get image from message
	cv_bridge::CvImageConstPtr image_ptr;
	cv::Mat image, image_grayscale, image_grayscale_small, image_display;
	convertImageMessageToMat(image_msg, image_ptr, image);
	if (image.channels() == 1)
	{
		// monochrome image
		image_grayscale = image.clone();
		cvtColor(image, image_display, CV_GRAY2BGR);
	}
	else if (image.channels() == 3)
	{
		// color image
		image_display = image.clone();
		cvtColor(image, image_grayscale, CV_BGR2GRAY);
	}
	else
	{
		ROS_ERROR("LabelReader::imageCallback: Wrong number of channels in camera image. Allowed is 1 or 3.");
		return;
	}
	cv::resize(image_grayscale, image_grayscale_small, cv::Size(tag_detection_target_image_width_, tag_detection_target_image_height_));

	// find text tags in original image
	std::vector<cv::Rect> detection_list;
	text_tag_detection_.text_tag_detection_fine_detection(image_grayscale_small, detection_list);
	std::cout<<detection_list.size()<<" text tags detected!"<<std::endl;
	std::cout << "Text Detection: [" << tim.getElapsedTimeInMilliSec() << " ms] processing time" << std::endl;

	// correct scale of detections
	const double factor_x = image.cols/(double)tag_detection_target_image_width_;
	const double factor_y = image.rows/(double)tag_detection_target_image_height_;
	for (size_t i=0; i<detection_list.size(); ++i)
	{
		detection_list[i].x *= factor_x;
		detection_list[i].width *= factor_x;
		detection_list[i].y *= factor_y;
		detection_list[i].height *= factor_y;
	}


	// read texts from tags
//	cvtColor(image, image, CV_GRAY2BGR);
//	image.convertTo(image, -1, 1.35, 0);
//	cv::Mat gray_image;
//	cv::cvtColor(image, gray_image, CV_BGR2GRAY);
//	gray_image.convertTo(gray_image, -1, 1.35, 0);

	for (size_t i=0; i< detection_list.size(); ++i)
	{
		if (detection_list[i].x!=0 && detection_list[i].y!=0)
		{
			cv::rectangle(image_display, detection_list[i], cv::Scalar(0,0,255), 3, 8, 0);

			std::cout<<"feature representation starts"<<std::endl;

			std::string tag_label_features, tag_label_template_matching;
			cv::Mat roi = image_grayscale(detection_list[i]);
			if (recognition_method_==2 || recognition_method_==3)
			{
				if (classifier_==2 || classifier_==3) // SVM for classification
				{
					// todo: check for const image
					tag_label_features=feature_representation_.read_text_tag_SVM(feature_representation_.numbers_svm,feature_representation_.letters_svm,roi,
																							classifier_,feature_number_,single_or_combination_);
				}

				else if (classifier_==1) // KNN for classification
				{
					tag_label_features=feature_representation_.read_text_tag_KNN(feature_representation_.numbers_knn,feature_representation_.letters_knn,roi,
																							classifier_,feature_number_,single_or_combination_);
				}
				else
					std::cout<<"[Read label ERROR] wrong *load* parameter given! "<<std::endl;

				cv::putText(image_display, tag_label_features, cv::Point(detection_list[i].x, detection_list[i].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(0,0,255), 2);
			}

			if (recognition_method_==1 || recognition_method_==3)
			{
				match_template_.read_tag(roi, tag_label_template_matching,template_matching_method_);
				std::cout << "The text tag reads: " << tag_label_template_matching << "." << std::endl;
				cv::putText(image_display, tag_label_template_matching, cv::Point(detection_list[i].x-100, detection_list[i].y-5), cv::FONT_HERSHEY_PLAIN, 2, CV_RGB(255,0,255), 2);
			}
		}
	}

	double time_in_mseconds = tim.getElapsedTimeInMilliSec();
	std::cout << "Whole system [" << time_in_mseconds << " ms] processing time" << std::endl;
	std::stringstream ss;
	ss<<time_in_mseconds<<"ms processing time";
	cv::putText(image_display, ss.str(), cv::Point(10, 30), cv::FONT_HERSHEY_PLAIN, 1.5, CV_RGB(255,0,0), 2);
	ss.str("");
	cv::imshow("image", image_display);
	cv::waitKey(1);
//	double roi_height,roi_width;
//	cv::Rect roi_rect((color_image.cols-roi_width)/2,(color_image.rows-roi_height)/2,roi_width,roi_height);
//	cv::rectangle(color_image, cv::Point(roi_rect.x-2, roi_rect.y-2), cv::Point(roi_rect.x+roi_rect.width+2, roi_rect.y+roi_rect.height+2), CV_RGB(0,0,255), 2);
//	cv::Mat gray_image;
//	cv::cvtColor(color_image, gray_image, CV_BGR2GRAY);
//	cv::Mat roi = gray_image(roi_rect);
//
//	// detect text
//
//	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
//	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
//
//	std::cout << "The text tag reads: " << tag_label << "." << std::endl;
//	cv::putText(color_image, tag_label, cv::Point(roi_rect.x-100, roi_rect.y-5), cv::FONT_HERSHEY_PLAIN, 3, CV_RGB(0,0,255), 2);
//	std::stringstream ss;
//	ss << roi_height_;
//	cv::putText(color_image, ss.str(), cv::Point(roi_rect.x+roi_rect.width+10, roi_rect.y+roi_rect.height), cv::FONT_HERSHEY_PLAIN, 1, CV_RGB(255,0,0), 1);
//
//	// now do something with color_image
//	cv::imshow("image", color_image);
//	char c = cv::waitKey(10);
//	if (c=='x')
//		roi_height_ += 1.0;
//	if (c=='y')
//		roi_height_ = std::max(10.0, roi_height_-1.0);
//
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

//template <typename T>
//void LabelReader::inputCallback(const sensor_msgs::PointCloud2::ConstPtr& point_cloud_msg)
//{
////		Timer tim;
////		tim.start();
//
//	// check camera link orientation and decide whether image must be turned around
//	bool turnAround = false;
//	tf::StampedTransform transform;
//	try
//	{
//		transform_listener_->lookupTransform("/base_link", "/head_cam3d_link", ros::Time(0), transform);
//		tfScalar roll, pitch, yaw;
//		transform.getBasis().getRPY(roll, pitch, yaw, 1);
//		if (cob3Number_ == 2)
//			roll *= -1.;
//		if (roll > 0.0)
//			turnAround = true;
//		//      std::cout << "xyz: " << transform.getOrigin().getX() << " " << transform.getOrigin().getY() << " " << transform.getOrigin().getZ() << "\n";
//		//      std::cout << "abcw: " << transform.getRotation().getX() << " " << transform.getRotation().getY() << " " << transform.getRotation().getZ() << " " << transform.getRotation().getW() << "\n";
//		//      std::cout << "rpy: " << roll << " " << pitch << " " << yaw << "\n";
//	} catch (tf::TransformException ex)
//	{
//		if (display_warnings_ == true)
//			ROS_WARN("%s",ex.what());
//	}
//
//	if (turnAround == false)
//	{
//		// image upright
//		//sensor_msgs::Image color_image_turned_msg = *color_image_msg;
//		//color_image_turned_msg.header.stamp = ros::Time::now();
//		//color_camera_image_pub_.publish(color_image_turned_msg);
//		//sensor_msgs::PointCloud2::ConstPtr point_cloud_turned_msg = point_cloud_msg;
//		point_cloud_pub_.publish(point_cloud_msg);
//	}
//	else
//	{
//		// image upside down
//		// point cloud
//		pcl::PointCloud<T> point_cloud_src;
//		pcl::fromROSMsg(*point_cloud_msg, point_cloud_src);
//
//		boost::shared_ptr<pcl::PointCloud<T> > point_cloud_turned(new pcl::PointCloud<T>);
//		point_cloud_turned->header = point_cloud_msg->header;
//		point_cloud_turned->height = point_cloud_msg->height;
//		point_cloud_turned->width = point_cloud_msg->width;
//		//point_cloud_turned->sensor_orientation_ = point_cloud_msg->sensor_orientation_;
//		//point_cloud_turned->sensor_origin_ = point_cloud_msg->sensor_origin_;
//		point_cloud_turned->is_dense = true;	//point_cloud_msg->is_dense;
//		point_cloud_turned->resize(point_cloud_src.height*point_cloud_src.width);
//		for (int v = (int)point_cloud_src.height - 1; v >= 0; v--)
//		{
//			for (int u = (int)point_cloud_src.width - 1; u >= 0; u--)
//			{
//				(*point_cloud_turned)(point_cloud_src.width-1 - u, point_cloud_src.height-1 - v) = point_cloud_src(u, v);
//			}
//		}
//
//		// publish turned data
//		sensor_msgs::PointCloud2::Ptr point_cloud_turned_msg(new sensor_msgs::PointCloud2);
//		pcl::toROSMsg(*point_cloud_turned, *point_cloud_turned_msg);
//		//point_cloud_turned_msg->header.stamp = ros::Time::now();
//		point_cloud_pub_.publish(point_cloud_turned_msg);
//
//		//      cv::namedWindow("test");
//		//      cv::imshow("test", color_image_turned);
//		//      cv::waitKey(10);
//	}
//
//	if (display_timing_ == true)
//		ROS_INFO("%d ImageFlip: Time stamp of pointcloud message: %f. Delay: %f.", point_cloud_msg->header.seq, point_cloud_msg->header.stamp.toSec(), ros::Time::now().toSec()-point_cloud_msg->header.stamp.toSec());
////		ROS_INFO("Pointcloud callback in image flip took %f ms.", tim.getElapsedTimeInMilliSec());
//}
//
//void LabelReader::imgConnectCB(const image_transport::SingleSubscriberPublisher& pub)
//{
//	img_sub_counter_++;
//	if (img_sub_counter_ == 1)
//	{
//		ROS_DEBUG("connecting");
//		color_camera_image_sub_.subscribe(*it_, "colorimage_in", 1);
//	}
//}
//
//void LabelReader::imgDisconnectCB(const image_transport::SingleSubscriberPublisher& pub)
//{
//	img_sub_counter_--;
//	if (img_sub_counter_ == 0) {
//		ROS_DEBUG("disconnecting");
//		color_camera_image_sub_.unsubscribe();
//	}
//}
//
//void LabelReader::pcConnectCB(const ros::SingleSubscriberPublisher& pub)
//{
//	pc_sub_counter_++;
//	if (pc_sub_counter_ == 1)
//	{
//		ROS_DEBUG("connecting");
//		if (pointcloud_data_format_.compare("xyz") == 0)
//			point_cloud_sub_ = node_handle_.subscribe<sensor_msgs::PointCloud2>("pointcloud_in", 1, &LabelReader::inputCallback<pcl::PointXYZ>, this);
//		else if (pointcloud_data_format_.compare("xyzrgb") == 0)
//			point_cloud_sub_ = node_handle_.subscribe<sensor_msgs::PointCloud2>("pointcloud_in", 1, &LabelReader::inputCallback<pcl::PointXYZRGB>, this);
//		else {
//			ROS_ERROR("Unknown pointcloud format specified in the paramter file.");
//			pc_sub_counter_ = 0;
//		}
//	}
//}
//
//void LabelReader::pcDisconnectCB(const ros::SingleSubscriberPublisher& pub)
//{
//	pc_sub_counter_--;
//	if (pc_sub_counter_ == 0)
//	{
//		ROS_DEBUG("disconnecting");
//		point_cloud_sub_.shutdown();
//	}
//}
