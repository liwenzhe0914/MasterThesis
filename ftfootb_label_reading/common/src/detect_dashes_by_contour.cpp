#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include "opencv2/objdetect/objdetect.hpp"
#include <dirent.h>
#include <stdio.h>
#include <string>
#include <math.h>
#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"

//operator to find the Rect with larger area
struct byArea
{
    bool operator () (const cv::Rect & a,const cv::Rect & b)
    {
         return a.width*a.height > b.width*b.height ;
    }
};

struct byCenterX
{
    bool operator () (const cv::Rect & a,const cv::Rect & b)
    {
         return a.x+0.5*a.width> b.x+0.5*b.width ;
    }
};


static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

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

// function to find right dashes.
// 1. sort Rect by area from large to small
// 2. too see if the rect comtains the center of the nonIntersect_dashes_list. If not, add this rect to nonIntersect_dashes_list
// 3. also check if y axis of center is at the same straight line
std::vector<cv::Rect> find_right_dashes(std::vector<cv::Rect> detected_dashes_list)
{
	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byArea());
	std::vector <cv::Rect> nonIntersect_dashes_list;
	for(unsigned int i=0; i < detected_dashes_list.size(); i++) {
	    bool toAdd = true;
	    cv::Point center = (detected_dashes_list[i].tl()+detected_dashes_list[i].br())*0.5;
	    for(unsigned int j=0; j < nonIntersect_dashes_list.size(); j++)
	        if (nonIntersect_dashes_list[j].contains(center))
	        {
	            toAdd = false;
	            break;
	        }
	    if (toAdd)
	        nonIntersect_dashes_list.push_back(detected_dashes_list[i]);
	 }
	std::sort(nonIntersect_dashes_list.begin(), nonIntersect_dashes_list.end(), byCenterX());
	return nonIntersect_dashes_list;
}

void unsharpMask(cv::Mat& im)
{
    cv::Mat tmp;
    cv::GaussianBlur(im, tmp, cv::Size(5,5), 0);
    cv::addWeighted(im, 1.5, tmp, -0.5, 0, im);
}

int main(int argc, char** argv)
{
	std::vector<std::string> ImageNames = load_folder_of_image(argv[1]);

	for (unsigned int k=0;k<ImageNames.size();k++)
	{

	cv::Mat src = cv::imread(ImageNames[k]);
	cv::Mat src0=src.clone();
	double resize_fx = 600. / src.cols;
	double resize_fy = 150. / src.rows;
	cv::resize(src,src,cv::Size(),resize_fx,resize_fy);

	cv::Mat gray1,gray2,gray3,gray4;
	cv::Mat bw1,bw2,bw3,bw4;
	cv::Mat src1,src2,src3,src4;
	//check if the image is empty
	if (src.empty())
		return -1;
	src.convertTo(src, -1, 1.5, 0);
//	while(1)
//		{
	std::vector<cv::Rect> detected_dashes_list;

	cv::Mat dst = src0.clone();

	//1. normal image 2. Gaussian blur 3. sharpen image 4. very bright image
	src.convertTo(src1, -1, 1.45, 0);
	cv::GaussianBlur(src, src2, cv::Size(3,3),0);
	src3=src.clone();
	unsharpMask(src3);
	src.convertTo(src4, -1, 2.85, 0);
	cv::cvtColor(src3, src3, CV_BGR2GRAY);
	cv::cvtColor(src2, src2, CV_BGR2GRAY);
	cv::cvtColor(src1, src1, CV_BGR2GRAY);
	cv::cvtColor(src4, src4, CV_BGR2GRAY);

	// Use Canny instead of threshold to catch squares with gradient shading on 3 conditions

	double CannyAccThresh = cv::threshold(src1,gray1,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Canny(src1,bw1,0,CannyAccThresh*1);

	CannyAccThresh = cv::threshold(src2,gray2,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Canny(src2,bw2,0,CannyAccThresh*1);

	CannyAccThresh = cv::threshold(src3,gray3,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Canny(src3,bw3,0,CannyAccThresh*1);

	CannyAccThresh = cv::threshold(src4,gray4,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Canny(src4,bw4,0,CannyAccThresh*1);

	// Find contours on 3 conditions and combine them
	std::vector<std::vector<cv::Point> > contours,contours1,contours2,contours3,contours4;
	cv::findContours(bw1, contours1, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw2, contours2, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw3, contours3, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw4, contours4, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
	//Most of the intersections can be filtered with CV_RETR_EXTERNAL flag.
	contours.reserve( contours1.size() + contours2.size() + contours3.size()+ contours4.size()); // preallocate memory for contours vector
	contours.insert( contours.end(), contours1.begin(), contours1.end() );
	contours.insert( contours.end(), contours2.begin(), contours2.end() );
	contours.insert( contours.end(), contours3.begin(), contours3.end() );
	contours.insert( contours.end(), contours4.begin(), contours4.end() );

//		std::cout<<contours.size()<<" "<<contours1.size()<<" "<<contours2.size()<<" "<<contours3.size()<<std::endl;
	std::vector<cv::Point> approx;


	for (unsigned int i = 0; i < contours.size(); i++)
	{
		// Approximate contour with accuracy proportional
		// to the contour perimeter
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.03, true);

		// Skip small or non-convex objects
		if (std::fabs(cv::contourArea(contours[i])) < 3)
			continue;

		else if (approx.size() >= 4 && approx.size() <= 6)
		{
			// Number of vertices of polygonal curve
			int vtc = approx.size();

			// Get the cosines of all corners
			std::vector<double> cos;
			for (int j = 2; j < vtc+1; j++)
				cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

			// Sort ascending the cosine values
			std::sort(cos.begin(), cos.end());

			// Get the lowest and the highest cosine
			double mincos = cos.front();
			double maxcos = cos.back();

			// Use the degrees obtained above and the number of vertices
			// to determine the shape of the contour
			if (vtc == 4 && mincos >= -0.23 && maxcos <= 0.3)
			{
				cv::Rect rect = cv::boundingRect(contours[i]);
				cv::Rect rect_original_size;
				rect_original_size.x=rect.x/resize_fx;
				rect_original_size.y=rect.y/resize_fy;
				rect_original_size.width=rect.width/resize_fx;
				rect_original_size.height=rect.height/resize_fy;
				detected_dashes_list.push_back(rect_original_size);
			}
		}
	}
	detected_dashes_list=find_right_dashes(detected_dashes_list);
	for (unsigned int i = 0; i < detected_dashes_list.size(); i++)
	{
		cv::Rect r = detected_dashes_list[i];
		cv::rectangle(dst, cv::Point(r.x,r.y),cv::Point(r.x+r.width,r.y+r.height), CV_RGB(255,0,255));
//		std::cout<<r.x+0.5*r.width<<" "<<r.y+0.5*r.height<<std::endl;
	}



	cv::imshow("src", src0);
	cv::imshow("dst", dst);
	cv::waitKey(0);
//		int c = cv::waitKey(0);
//		if (c=='y')
//			key1=key1+0.05;
//		else if (c=='x')
//			key1=key1-0.05;
//		else if (c=='a')
//			key2=key2+0.05;
//		else if (c=='s')
//			key2=key2-0.05;
//		else if (c=='q')
//			break;
//		std::cout<<"key1= "<<key1<<std::endl;
//		std::cout<<"key2= "<<key2<<std::endl;
//	}
	}

	return 0;
}
