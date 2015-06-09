// OpenCV includes
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Boost includes
//#include <boost/filesystem.hpp>
//#include "boost/filesystem/operations.hpp"
//#include "boost/filesystem/fstream.hpp"


//ros includes
#include <ros/ros.h>
#include <ros/package.h>

// Different includes
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>

int main (int argc, char** argv)
{
	std::string folder=argv[1];
	std::string saving_directory="/home/damon/training_samples_for_SVM/";
	std::string saving_directory1="/home/damon/training_samples_for_SVM/letters/";
	std::string saving_directory2="/home/damon/training_samples_for_SVM/numbers/";
	std::vector<std::string> ImageNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(folder.c_str())) != NULL)
	{
		while ((entry = readdir(pDIR)) != NULL)
		{
			if (std::strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				std::string completeName = entry->d_name;
				std::string imgend = completeName.substr(completeName.find_last_of(".") + 1, completeName.length() - completeName.find_last_of("."));
				if (std::strcmp(imgend.c_str(), "png") == 0 || std::strcmp(imgend.c_str(), "PNG") == 0
						|| std::strcmp(imgend.c_str(), "JPG") == 0 || std::strcmp(imgend.c_str(), "jpg") == 0
						|| std::strcmp(imgend.c_str(), "jpeg") == 0 || std::strcmp(imgend.c_str(), "Jpeg") == 0
						|| std::strcmp(imgend.c_str(), "bmp") == 0 || std::strcmp(imgend.c_str(), "BMP") == 0
						|| std::strcmp(imgend.c_str(), "TIFF") == 0 || std::strcmp(imgend.c_str(), "tiff") == 0
						|| std::strcmp(imgend.c_str(), "tif") == 0 || std::strcmp(imgend.c_str(), "TIF") == 0)
				{
					std::string s = folder;
					if (s.at(s.length() - 1) != '/')
						s.append("/");
					s.append(entry->d_name);
					ImageNames.push_back(s);
					//std::cout << "imagename: " << s << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
	for (unsigned int i = 0; i < ImageNames.size(); i++)
		{
			cv::Mat original_image;
			int lastindex = ImageNames[i].find_last_of(".");
			int lastindex2 = ImageNames[i].find_last_of("/");
			std::string rawname = ImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
			original_image= cv::imread(ImageNames[i],CV_LOAD_IMAGE_GRAYSCALE);
			//std::cout<<ImageNames[i]<<std::endl;
			//cv::namedWindow( "Display window", CV_WINDOW_AUTOSIZE );
			//cv::imshow( "Display window", original_image );
			cv::Mat sub_image = original_image(cv::Rect(original_image.cols*0,0, original_image.cols*0.25, original_image.rows));
			cv::namedWindow( "Display subimages", CV_WINDOW_AUTOSIZE );
			cv::imshow( "Display subimages", sub_image );
			//cv::waitKey(0);
			std::stringstream ss;
			ss<<saving_directory1<<rawname<<"_"<<0/0.25+1<<".png";
			std::string name = ss.str();
			ss.str("");
			cv::imwrite(name,sub_image);
			for (double x_min_ratio = 0.25; x_min_ratio < 1.0; x_min_ratio += 0.25)
			{
				cv::Mat sub_image = original_image(cv::Rect(original_image.cols*x_min_ratio,0, original_image.cols*0.25, original_image.rows));
				cv::namedWindow( "Display subimages", CV_WINDOW_AUTOSIZE );
				cv::imshow( "Display subimages", sub_image );
				//cv::waitKey(0);
				std::stringstream ss;
				ss<<saving_directory2<<x_min_ratio<<"/"<<rawname<<"_"<<x_min_ratio/0.25+1<<".png";
				std::string name = ss.str();
				ss.str("");
				cv::imwrite(name,sub_image);

			}
		}

}
