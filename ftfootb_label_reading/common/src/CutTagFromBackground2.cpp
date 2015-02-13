#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <math.h>
#include <map>

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <vector>
#include <iostream>

using namespace cv;
using namespace std;

std::vector<std::string> load_folder_of_image(std::string path)
{
	std::vector<std::string> ImageNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
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
					std::string s = path;
					if (s.at(s.length() - 1) != '/')
						s.append("/");
					s.append(entry->d_name);
					ImageNames.push_back(s);
//					std::cout << "imagename: " << s << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
return ImageNames;
}

int count_white_pixels(Mat dst,Point pt1,Point pt2,bool vertical)
{
	int count=0;
	Point temp;
	if (vertical)
	{
//		cout<<"yx"<<pt1.y<<" "<<pt2.y<<endl;
//		cout<<"vertical x"<<pt1.x<<" "<<pt2.x<<endl;
		for(temp.y=-200;temp.y<200;temp.y++)
			{
			if (dst.at<uchar>(temp.y,pt2.x)==(255))
			{
				count++;
			}
			}
	}
	else
	{
//		cout<<"horizontal y"<<pt1.y<<" "<<pt2.y<<endl;
		for(temp.x=pt1.x;temp.x<pt2.x;temp.x++)
		{
//			cout<<pt1.y<<" "<<temp.x<<endl;
			if (dst.at<uchar>(pt1.y, temp.x)==(255))
			{
				count++;
			}
		}
	}
return count;
}

float find_best_two_lines(std::map<float,float> lines_with_count_map)
{

	unsigned currentMax = 0;
	unsigned arg_max = 0;
	std::map<float,float>::iterator it = lines_with_count_map.begin();
	for(it=lines_with_count_map.begin(); it!=lines_with_count_map.end(); ++it )
	    {
		if (it->first > currentMax)
	    	{
	        	arg_max = it->second;
	        	currentMax = it->first;
	    	}
	    }
//	cout << "line " << arg_max << " have " << currentMax << " count as max" << endl;
	return arg_max;
}

void help()
{
 cout << "\nThis program demonstrates line finding with the Hough transform.\n"
         "Usage:\n"
         "./cut_tag_from_background <image_name>, Default is pic1.jpg\n" << endl;
}

Rect get_rect_with_hough_line_transform(Mat src)
{
	double resize_fx = 400. / src.cols;
	double resize_fy = 100. / src.rows;
	Rect rectangle;
	std::map<float,float> upper_horizontal_with_count_map,lower_horizontal_with_count_map,
	left_vertical_with_count_map,right_vertical_with_count_map;
	Mat dst,cdst;
	resize (src,src,cv::Size(),resize_fx,resize_fy);
	src.convertTo(src, -1, 0.8, 0);
	Mat Img_Thres_Gray;
	double CannyAccThresh = threshold(src,Img_Thres_Gray,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	double CannyThresh = 0.1 * CannyAccThresh;
	Canny(src,dst,0,CannyAccThresh/2);
//	Canny(src, dst, 100, 200, 3);
//	imshow("source", dst);
	waitKey(0);
	cvtColor(dst, cdst, CV_GRAY2BGR);
	vector<Vec2f> lines_horizontal;
	HoughLines(dst, lines_horizontal, 1, CV_PI/180, 40, 0, 0 );
	for( size_t i = 0; i < lines_horizontal.size(); i++ )
		{
		 float rho = lines_horizontal[i][0], theta = lines_horizontal[i][1];

		if(theta>CV_PI/180*89.5 && theta<CV_PI/180*90.5)
		{
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			line( cdst, pt1, pt2, Scalar(255,0,255), 3, CV_AA);
			int count=count_white_pixels(dst,pt1,pt2,0);
			if (pt1.y<0.25*cdst.rows)
			{
			upper_horizontal_with_count_map[count]=pt1.y;
			}
			else if (pt1.y>0.75*cdst.rows)
			{
			lower_horizontal_with_count_map[count]=pt1.y;
			}
		}
		}

		vector<Vec2f> lines_vertical;

		HoughLines(dst, lines_vertical, 1, CV_PI/180, 20, 0, 0 );
		for( size_t i = 0; i < lines_vertical.size(); i++ )
		{
			float rho = lines_vertical[i][0], theta = lines_vertical[i][1];
			if(theta>CV_PI/180*179.5 || theta<CV_PI/180*0.5)
			{
				Point pt1, pt2;
				double a = cos(theta), b = sin(theta);
				double x0 = a*rho, y0 = b*rho;
				pt1.x = cvRound(x0 + 1000*(-b));
				pt1.y = cvRound(y0 + 1000*(a));
				pt2.x = cvRound(x0 - 1000*(-b));
				pt2.y = cvRound(y0 - 1000*(a));
				line( cdst, pt1, pt2, Scalar(255,0,255), 3, CV_AA);
				int count=count_white_pixels(dst,pt1,pt2,1);
//					cout<<count<<endl;
				if (pt1.x>0.92*cdst.cols)
				{
					right_vertical_with_count_map[count]=pt1.x;
				}
				else if (pt1.x<0.08*cdst.cols)
				{
					left_vertical_with_count_map[count]=pt1.x;
				}
			}
		}
		float best_left_vertical = find_best_two_lines(left_vertical_with_count_map);
		float best_right_vertical = find_best_two_lines(right_vertical_with_count_map);
		float best_upper_horizontal = find_best_two_lines(upper_horizontal_with_count_map);
		float best_lower_horizontal = find_best_two_lines(lower_horizontal_with_count_map);
		if (best_lower_horizontal==0)
		{
			best_lower_horizontal=100;
		}
		if (best_right_vertical==0)
		{
			best_right_vertical=400;
		}
		rectangle.x=best_left_vertical/resize_fx;
		rectangle.y=best_upper_horizontal/resize_fy;
		rectangle.width=best_right_vertical/resize_fx-best_left_vertical/resize_fx;
		rectangle.height=best_lower_horizontal/resize_fy-best_upper_horizontal/resize_fy;
//		cout<<best_left_vertical<<" "<<best_right_vertical<<" "<<best_upper_horizontal<<" "<<best_lower_horizontal<<endl;
		cout<<rectangle.x<<" "<<rectangle.y<<" "<<rectangle.width<<" "<<rectangle.height<<endl;

		waitKey(0);
return rectangle;
}

int main(int argc, char** argv)
{
	string path =argv[1];
	std::vector<std::string> ImageNames = load_folder_of_image(path);
	for (unsigned int i = 0;i<ImageNames.size();i++)
	{
		cout<<ImageNames[i]<<" ";
	Mat src = imread(ImageNames[i], 0);
	Rect rectangle_info=get_rect_with_hough_line_transform(src);
	cvtColor(src, src, CV_GRAY2BGR);
	Mat img=imread(ImageNames[i]);
	rectangle(img,rectangle_info, Scalar(0,0,255),3,8,0);
	imshow("source", src);
	imshow("detected lines", img);

	 waitKey(0);
 }
}

