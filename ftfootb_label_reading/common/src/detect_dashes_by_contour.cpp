#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>
#include "opencv2/objdetect/objdetect.hpp"

/**
 * Helper function to find a cosine of angle between vectors
 * from pt0->pt1 and pt0->pt2
 */
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/**
 * Helper function to display text in the center of a contour
 */

void find_right_dashes(std::vector<cv::Rect> detected_dashes_list)
{
	std::vector<cv::Point> dashes_centers;
	for (unsigned int i = 0; i < detected_dashes_list.size(); i++)
	{
		cv::Point center;
		cv::Rect r;
		center.x=r.x+0.5*r.width;
		center.y=r.y+0.5*r.height;
		dashes_centers.push_back(center);
	}
}

void unsharpMask(cv::Mat& im)
{
    cv::Mat tmp;
    cv::GaussianBlur(im, tmp, cv::Size(5,5), 0);
    cv::addWeighted(im, 1.5, tmp, -0.5, 0, im);
}

int main(int argc, char** argv)
{
	cv::Mat src = cv::imread(argv[1]);

	cv::resize(src,src,cv::Size(600,150));
	cv::Mat gray1,gray2,gray3,gray4;
	cv::Mat bw1,bw2,bw3,bw4;
	cv::Mat src1,src2,src3,src4;
	//check if the image is empty
	if (src.empty())
		return -1;
	src.convertTo(src, -1, 1.5, 0);
	double key1=1,key2=0.2;

	while (1)
	{
		std::vector<cv::Rect> detected_dashes_list;
	cv::Mat dst = src.clone();
	//1. normal image 2. Gaussian blur 3. sharpen image 4. very bright image
	src.convertTo(src1, -1, 1.5, 0);
	cv::GaussianBlur(src, src2, cv::Size(3,3),0);
	src3=src.clone();
	unsharpMask(src3);
	src.convertTo(src4, -1, 2.8, 0);
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
	cv::findContours(bw1, contours1, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw2, contours2, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw3, contours3, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	cv::findContours(bw4, contours4, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
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
				cv::Rect r = cv::boundingRect(contours[i]);
				detected_dashes_list.push_back(r);
			}
		}
	}

	for (unsigned int i = 0; i < detected_dashes_list.size(); i++)
	{
		cv::Rect r = detected_dashes_list[i];
		cv::rectangle(dst, cv::Point(r.x,r.y),cv::Point(r.x+r.width,r.y+r.height), CV_RGB(255,0,255));
	}
	cv::imshow("src", src);
	cv::imshow("dst", dst);
//	cv::waitKey(0);
		int c = cv::waitKey(0);
		if (c=='y')
			key1=key1+1;
		else if (c=='x')
			key1=key1-1;
		else if (c=='a')
			key2=key2+0.05;
		else if (c=='s')
			key2=key2-0.05;
		std::cout<<"key1= "<<key1<<std::endl;
		std::cout<<"key2= "<<key2<<std::endl;
	}


	return 0;
}
