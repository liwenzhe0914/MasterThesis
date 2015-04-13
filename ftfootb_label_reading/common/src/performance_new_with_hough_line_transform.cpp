//This program is modified from original performance.cpp to give the performance of new cascade classifier and
//also save the detected text tags' infomation into a xml file like following format.
//<?xml version="1.0" encoding="UTF-8"?>
//<tagset>
//  <image>
//    <imageName>images/image1.jpg</imageName>
//   <taggedRectangles>
//     <taggedRectangle x="1276" y="900" width="193" height="61" modelType="17"  />
//      <taggedRectangle x="348" y="844" width="197" height="105" modelType="3"  />
//      <taggedRectangle x="776" y="812" width="281" height="165" modelType="42"  />
//    </taggedRectangles>
//  </image>
//  <image>
//    <imageName>images/image2.jpg</imageName>
//    <taggedRectangles>
//      <taggedRectangle x="1036" y="972" width="75" height="29" modelType="1"  />
//      <taggedRectangle x="110" y="964" width="411" height="45" modelType="2"  />
//      <taggedRectangle x="1126" y="966" width="441" height="59" modelType="3"  />
//      <taggedRectangle x="604" y="964" width="349" height="53" modelType="2"  />
//      <taggedRectangle x="262" y="766" width="595" height="67" modelType="17"  />
//      <taggedRectangle x="948" y="766" width="461" height="71" modelType="36"  />
//      <taggedRectangle x="366" y="318" width="57" height="29" modelType="14"  />
//      <taggedRectangle x="598" y="844" width="411" height="89" modelType="11"  />
//      <taggedRectangle x="828" y="388" width="181" height="45" modelType="11"  />
//      <taggedRectangle x="1004" y="292" width="197" height="109" modelType="14"  />
//      <taggedRectangle x="844" y="8" width="581" height="233" modelType="31"  />
//    </taggedRectangles>
//  </image>
//</tagset>
//

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <time.h>
#include <dirent.h>
#include <math.h>
#include <map>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <vector>
#include <algorithm>


using namespace cv;
using namespace std;


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


// function to find right dashes.
// 1. sort Rect by area from large to small
// 2. too see if the rect contains the center of the nonIntersect_dashes_list. If not, add this rect to nonIntersect_dashes_list
// 3. also make sure width > height
// 4. also check if y axis of center is at the same straight line
std::vector<cv::Rect> find_right_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Mat img,cv::Rect rect)
{
	cv::Mat roi = img(rect);
	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byArea());
	std::vector <cv::Rect> nonIntersect_dashes_list;
	for(unsigned int i=0; i < detected_dashes_list.size(); i++)
	{
	    bool toAdd = true;
	    cv::Point center = (detected_dashes_list[i].tl()+detected_dashes_list[i].br())*0.5;
	    if (detected_dashes_list[i].height>=detected_dashes_list[i].width)
			toAdd = false;

	    if (double(detected_dashes_list[i].width*detected_dashes_list[i].height)>=0.02*double(rect.width*rect.height))
			toAdd = false;

	    if ((center.y-rect.y)>0.85*roi.rows||(center.y-rect.y)<0.15*roi.rows)
	   			toAdd = false;



	    if (double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)<1.5||
	    			double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)>4)
	    		toAdd = false;


//	    if (double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)<1.5||
//	    			double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)>3.9)
//	    		toAdd = false;


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


//	if (nonIntersect_dashes_list.size()>3)
//	{
//		double sum_y=0;
//		for (unsigned int i = 0; i < nonIntersect_dashes_list.size(); i++)
//		{
//			cv::Rect r = nonIntersect_dashes_list[i];
//			sum_y=sum_y+double(r.y+0.5*r.height);
//		}
//			double mean_y= sum_y/nonIntersect_dashes_list.size();
//			std::cout<<"mean_y:"<<mean_y<<std::endl;
//
//	}
//	else if (nonIntersect_dashes_list.size()==0)
//		std::cout<<"No dash detected."<<std::endl;
//	else if (nonIntersect_dashes_list.size()==2)
//		{
//			cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
//			cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
//			if (std::fabs(double(center0.y)-double(center1.y))>1/25*src.rows)
//			{
//				std::vector <cv::Rect> nonIntersect_dashes_list;
//			}
//		}
	return nonIntersect_dashes_list;
}

cv::Rect restore_text_tag_by_three_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img)
{

	//cv::Mat roi=img(text_tag);
	cv::Rect text_tag_original=text_tag;
	cout<<"cj"<<endl;
	cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
	cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
	cv::Point center2 = (detected_dashes_list[2].tl()+detected_dashes_list[2].br())*0.5;

	cv::Point tag_center;
	double two_near_dashes_gap = std::fabs( 0.5*(double(detected_dashes_list[2].x) - double(detected_dashes_list[0].x)) );
	tag_center.x=ceil((double(center1.x-text_tag.x)-2./140.*two_near_dashes_gap)+text_tag.x);
	tag_center.y=ceil((97./53.*double(center1.y-text_tag.y)*0.5)+text_tag.y);

	text_tag.width = ceil(690./179.5*two_near_dashes_gap);//two_near_dashes_gap*2 + 127* two_near_dashes_gap/140+ 123* two_near_dashes_gap/140;
	text_tag.height = ceil(129./640.*text_tag.width);
	text_tag.x= center1.x-0.5*text_tag.width;
	text_tag.y= center1.y-6.5/129.*text_tag.height-0.5*text_tag.height;
	std::cout<<"text_tag: "<<text_tag.x<<" "<<text_tag.y<<" "<<text_tag.width<<" "<<text_tag.height<<std::endl;
	if(text_tag.x<0 || text_tag.y<0 || (text_tag.x+text_tag.width)>img.cols || (text_tag.y+text_tag.height)>img.rows )
	{
		text_tag.x =0;
		text_tag.y =0;
		text_tag.width=1;
		text_tag.height=1;
	}

	return text_tag;
}

cv::Rect select_best_match_from_three_estimated_dashes(cv::Rect text_tag_l,cv::Rect text_tag_m,cv::Rect text_tag_r,cv::Rect text_tag,cv::Mat img)
{
	int match_method=CV_TM_CCOEFF_NORMED;//CV_TM_CCOEFF_NORMED
	cv::Rect best_text_tag;
	cv::Mat result_r,result_l,result_m;
	cv::Mat template_image = cv::imread("/home/damon/git/care-o-bot/ftfootb/ftfootb_label_reading/common/files/template.png",CV_LOAD_IMAGE_GRAYSCALE);
	cv::cvtColor(img, img, CV_BGR2GRAY);
	bool left=true,middle=true,right=true;
	if (text_tag_l.width*text_tag_l.height<5)
//	if (text_tag_l.width*text_tag_l.height<5 || text_tag_l.height>0.5*text_tag.height || text_tag_l.height<0.5*text_tag.height)
		left=false;
//	if (text_tag_m.width*text_tag_m.height<5 )
	if (text_tag_m.width*text_tag_m.height<5 || text_tag_m.width<0.6*text_tag.width)
		middle=false;
	if (text_tag_r.width*text_tag_r.height<5)
//	if (text_tag_r.width*text_tag_r.height<5 || text_tag_r.height>0.5*text_tag.height || text_tag_r.height<0.5*text_tag.height)
		right=false;
	cv::Mat source_image_l=img(text_tag_l);
	cv::Mat source_image_r=img(text_tag_r);
	cv::Mat source_image_m=img(text_tag_m);
	cv::Size dsize(55,10);
	cv::resize(template_image,template_image,dsize);
	cv::resize(source_image_l,source_image_l,dsize);
	cv::resize(source_image_m,source_image_m,dsize);
	cv::resize(source_image_r,source_image_r,dsize);
	double minVal, maxVal,score = 0,score_l = 0,score_m = 0,score_r = 0;
	cv::Point minLoc, maxLoc,matchLoc;
	matchTemplate(source_image_l,template_image,result_l,match_method);

	cv::minMaxLoc( result_l, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_l=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_l= 1.-maxVal;
	}
	matchTemplate(source_image_m,template_image,result_m,match_method);
	cv::minMaxLoc( result_m, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_m=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_m= 1.-maxVal;
	}
	matchTemplate(source_image_r,template_image,result_r,match_method);
	cv::minMaxLoc( result_r, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_r=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_r=1.- maxVal;
	}
	if (!left)
		score_l=10;
	if (!middle)
		score_m=10;
	if (!right)
		score_r=10;


cout<<"l "<<score_l<<" m "<<score_m<<" r "<<score_r<<" "<<endl;

std::map<double,string> map;
map[score_l]="l";
map[score_m]="m";
map[score_r]="r";
double min1 = std::min(score_m,score_r);
double minn = std::min(min1,score_l);
string best=map.find(minn) -> second;
cout<<"best:"<<best<<endl;
	if (best=="l")
		best_text_tag=text_tag_l;
	else if (best=="m")
		best_text_tag=text_tag_m;
	else if (best=="r")
		best_text_tag=text_tag_r;

	if (score_l==10 && score_m==10 && score_r==10)
		best_text_tag=text_tag;
/*	std::map<char,double> map;

	map['l']=score_l;
	map['m']=score_m;
	map['r']=score_r;

	score= map.begin() -> second;
	cout<<"score:"<<score<<endl;
	cout<<"best:"<<map.begin() -> first<<endl;
	char best=map.begin() -> first;
	if (best=='l')
		best_text_tag=text_tag_l;
	else if (best=='m')
			best_text_tag=text_tag_m;
	else if (best=='r')
			best_text_tag=text_tag_r;*/
	std::cout<<"best_text_tag: "<<best_text_tag.x<<" "<<best_text_tag.y<<" "<<best_text_tag.width<<" "<<best_text_tag.height<<std::endl;
	return best_text_tag;
}

cv::Rect restore_tag_by_estimated_dashes(cv::Point estimated_dash_center,cv::Rect text_tag,cv::Mat img,std::vector<cv::Rect> detected_dashes_list)
{

	Point r1, r2;
	r1.x = estimated_dash_center.x-0.5*detected_dashes_list[0].width;
	r1.y = estimated_dash_center.y-0.5*detected_dashes_list[0].height;
	r2.x = detected_dashes_list[0].width;
	r2.y = detected_dashes_list[0].height;

	cv::Rect rect_temp;
	rect_temp.x=r1.x;
	rect_temp.y=r1.y;
	rect_temp.width = r2.x;
	rect_temp.height = r2.y;

	detected_dashes_list.push_back(rect_temp);

	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byCenterX());
cout<<"detected_dashes_list: "<<detected_dashes_list[0].x<<detected_dashes_list[1].x<<detected_dashes_list[2].x<<endl;
	cv:: Rect text_tag_candidate=restore_text_tag_by_three_detected_dashes(detected_dashes_list,text_tag,img);


	return text_tag_candidate;
}

cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img)
{
	cv::Mat roi=img(text_tag);
	if (detected_dashes_list.size()==3 )
	{
		text_tag=restore_text_tag_by_three_detected_dashes(detected_dashes_list,text_tag,img);
	}
	else if (detected_dashes_list.size()==2)
	{
		cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
		cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
		std::vector<cv::Rect> detected_dashes_list_l=detected_dashes_list;
		std::vector<cv::Rect> detected_dashes_list_m=detected_dashes_list;
		std::vector<cv::Rect> detected_dashes_list_r=detected_dashes_list;

		cv::Point center_l=cv::Point (center1.x-(center0.x-center1.x),center0.y);
		cv::Point center_r=cv::Point (center0.x+(center0.x-center1.x),center0.y);
		cv::Point center_m=cv::Point (center0.x-0.5*(center0.x-center1.x),center0.y);


		cv::Rect text_tag_l,text_tag_m,text_tag_r;

		text_tag_l=restore_tag_by_estimated_dashes(center_l,text_tag,img,detected_dashes_list);
		text_tag_m=restore_tag_by_estimated_dashes(center_m,text_tag,img,detected_dashes_list);
		text_tag_r=restore_tag_by_estimated_dashes(center_r,text_tag,img,detected_dashes_list);

		std::cout<<"text_tag l: "<<text_tag_l.x<<" "<<text_tag_l.y<<" "<<text_tag_l.width<<" "<<text_tag_l.height<<std::endl;
		std::cout<<"text_tag r: "<<text_tag_r.x<<" "<<text_tag_r.y<<" "<<text_tag_r.width<<" "<<text_tag_r.height<<std::endl;
		std::cout<<"text_tag m: "<<text_tag_m.x<<" "<<text_tag_m.y<<" "<<text_tag_m.width<<" "<<text_tag_m.height<<std::endl;
		text_tag=select_best_match_from_three_estimated_dashes(text_tag_l,text_tag_m,text_tag_r,text_tag,img);
	}
	return text_tag;
}
//cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img)
//{
//	cv::Mat roi=img(text_tag);
//	if (detected_dashes_list.size()==3 )
////		&& (double(detected_dashes_list[0].y+0.5*detected_dashes_list[0].height)-
////											double(detected_dashes_list[1].y+0.5*detected_dashes_list[1].height))<roi.rows/25
////											&& std::fabs( double(detected_dashes_list[2].x)- double(detected_dashes_list[1].x)
////											- double(detected_dashes_list[1].x)-double(detected_dashes_list[0].x) )<roi.cols/25 )
//	{
//		cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
//		cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
//		cv::Point center2 = (detected_dashes_list[2].tl()+detected_dashes_list[2].br())*0.5;
//		std::cout<<"center0: "<<center0.x<<" "<<center0.y<<std::endl;
//		std::cout<<"center1: "<<center1.x<<" "<<center1.y<<std::endl;
//		std::cout<<"center2: "<<center2.x<<" "<<center2.y<<std::endl;
//		cv::Point tag_center;
//		double two_near_dashes_gap = std::fabs( 0.5*(double(detected_dashes_list[2].x) - double(detected_dashes_list[0].x)) );
//		tag_center.x=ceil((double(center1.x-text_tag.x)-2./140.*two_near_dashes_gap)+text_tag.x);
//		tag_center.y=ceil((97./53.*double(center1.y-text_tag.y)*0.5)+text_tag.y);
//		std::cout<<"tag_center.x "<<tag_center.x<<std::endl;
//		std::cout<<"tag_center.y "<<tag_center.y<<std::endl;
//		text_tag.width = ceil(53./14.*two_near_dashes_gap);//two_near_dashes_gap*2 + 127* two_near_dashes_gap/140+ 123* two_near_dashes_gap/140;
//		text_tag.height = ceil(97./140.*two_near_dashes_gap);
//		text_tag.x= tag_center.x-0.5*text_tag.width;
//		text_tag.y= tag_center.y-0.5*text_tag.height;
//
//		std::cout<<"text_tag: "<<text_tag.x<<" "<<text_tag.y<<" "<<text_tag.width<<" "<<text_tag.height<<std::endl;
//	}
//	return text_tag;
//}

void unsharpMask(cv::Mat& im)
{
    cv::Mat tmp;
    cv::GaussianBlur(im, tmp, cv::Size(5,5), 0);
    cv::addWeighted(im, 1.5, tmp, -0.5, 0, im);
}

std::vector<cv::Rect>detect_dashes (cv::Rect rect,cv::Mat img)
{
		cv::Mat src =img(rect);
		cv::Mat src0=src.clone();
		double resize_fx = 600. / src.cols;
		double resize_fy = 150. / src.rows;
		cv::resize(src,src,cv::Size(),resize_fx,resize_fy);

		cv::Mat gray1,gray2,gray3,gray4;
		cv::Mat bw1,bw2,bw3,bw4;
		cv::Mat src1,src2,src3,src4;
		//check if the image is empty
		if (src.empty())
			std::cout<<"[detect dashed ERROR] could not load the image."<<std::endl;
		src.convertTo(src, -1, 1.5, 0);
	//	while(1)
	//		{
		std::vector<cv::Rect> detected_dashes_list;

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

		//			transfer the dashes coordinates from roi to img and also show them
		std::vector<cv::Rect> detected_dashes_list_temp;
		for (unsigned int p = 0; p < detected_dashes_list.size(); p++)
			{

				cv::Rect r;
				r.x =detected_dashes_list[p].x + rect.x;
				r.y =detected_dashes_list[p].y + rect.y;
				r.width=detected_dashes_list[p].width;
				r.height=detected_dashes_list[p].height;
				detected_dashes_list_temp.push_back(r);
			}
		detected_dashes_list=find_right_dashes(detected_dashes_list_temp,img,rect);

		std::cout<<"detected_dashes_list size 0000=" <<detected_dashes_list.size() <<std::endl;

		return detected_dashes_list;

}

void help()
{
 cout << "\nThis program demonstrates line finding with the Hough transform.\n"
         "Usage:\n"
         "./cut_tag_from_background <image_name>, Default is pic1.jpg\n" << endl;
}

Rect get_rect_with_hough_line_transform(Mat src)
{
	cvtColor(src, src, CV_RGB2GRAY);
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
//	waitKey(0);
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

//		waitKey(0);
return rectangle;
}

#ifndef PATH_MAX
#define PATH_MAX 512
#endif /* PATH_MAX */

/*typedef struct HidCascade {
 int size;
 int count;
 } HidCascade;
 */

typedef struct ObjectPos
{
    float x;
    float y;
    float width;
    int found; /* for reference */
    int neghbors;
} ObjectPos;

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    int i, j;
    char* classifierdir = NULL;
    //char* samplesdir    = NULL;

    int saveDetected = 1;
    double scale_factor = 1.1;
    float maxSizeDiff = 1.5F;

    float maxPosDiff = 0.4F;

    /* number of stages. if <=0 all stages are used */
    //int nos = -1, nos0;
    int width = 125;
    int height = 18;

    int rocsize;

    FILE* info;
    FILE* resultados;
    FILE* detected_text_tags;
    char* infoname;
    char fullname[PATH_MAX];
    char detfilename[PATH_MAX];
    char* filename;
    char detname[] = "det-";

    CascadeClassifier cascade;
	std::vector<int> reject_levels;
	std::vector<double> level_weights;
    double totaltime;

    if (!(resultados = fopen("resultados.txt", "w")))
    {
        printf("Cannot create results file.\n");
        exit(-1);
    }

    if (!(detected_text_tags = fopen("detected_text_tags.xml", "w")))
    {
        printf("Cannot create xml file.\n");
        exit(-1);
    }

    infoname = (char*) "";
    rocsize = 20;
    if (argc == 1)
    {
        printf("Usage: %s\n  -data <classifier_directory_name>\n"
                "  -info <collection_file_name>\n"
                "  [-maxSizeDiff <max_size_difference = %f>]\n"
                "  [-maxPosDiff <max_position_difference = %f>]\n"
                "  [-sf <scale_factor = %f>]\n"
                "  [-ni]\n"
                "  [-rs <roc_size = %d>]\n"
                "  [-w <sample_width = %d>]\n"
                "  [-h <sample_height = %d>]\n", argv[0], maxSizeDiff,
                maxPosDiff, scale_factor, rocsize, width, height);

        return 0;
    }

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-data"))
        {
            classifierdir = argv[++i];
        }
        else if (!strcmp(argv[i], "-info"))
        {
            infoname = argv[++i];
        }
        else if (!strcmp(argv[i], "-maxSizeDiff"))
        {
            maxSizeDiff = (float) atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-maxPosDiff"))
        {
            maxPosDiff = (float) atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-sf"))
        {
            scale_factor = atof(argv[++i]);
        }
        else if (!strcmp(argv[i], "-ni"))
        {
            saveDetected = 0;
        }
        else if (!strcmp(argv[i], "-rs"))
        {
            rocsize = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-w"))
        {
            width = atoi(argv[++i]);
        }
        else if (!strcmp(argv[i], "-h"))
        {
            height = atoi(argv[++i]);
        }
    }

    if (!cascade.load(classifierdir)) {
        printf("Unable to load classifier from %s\n", classifierdir);
        return 1;
    }

    strcpy(fullname, infoname);
    filename = strrchr(fullname, '\\');
    if (filename == NULL) {
        filename = strrchr(fullname, '/');
    }
    if (filename == NULL) {
        filename = fullname;
    } else {
        filename++;
    }

    info = fopen(infoname, "r");
    totaltime = 0.0;

    if (info != NULL) {

        //int x, y, width, height;
    	int x, y;
        Mat img;
        int hits, missed, falseAlarms;
        int totalHits, totalMissed, totalFalseAlarms;
        int found;
        float distance;



        int refcount;
        ObjectPos* ref;
        int detcount;
        ObjectPos* det;
        int error = 0;

        int* pos;
        int* neg;

        pos = (int*) cvAlloc(rocsize * sizeof(*pos));
        neg = (int*) cvAlloc(rocsize * sizeof(*neg));
        for (i = 0; i < rocsize; i++)
        {
            pos[i] = neg[i] = 0;
        }

        printf("+================================+======+======+======+==========+==========+==========+==========+\n");
        printf("|            File Name           | Hits |Missed| False|retangle.x|retangle.y|retangle.w|retangle.h|\n");
        printf("+================================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
                "+===============================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
                "|            File Name          | Hits |Missed| False|retangle.x|retangle.y|retangle.w|retangle.h|\n");
        fprintf(resultados,
                "+===============================+======+======+======+==========+==========+==========+==========+\n");
        //fprintf (resultados, "%d\n",framesCnt);

        fprintf(detected_text_tags,"<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n<tagset>\n");

        totalHits = totalMissed = totalFalseAlarms = 0;
        while (!feof(info))
        {
            fscanf(info, "%s %d", filename, &refcount);
            img = imread(fullname);
            cv::Mat img_copy=img.clone();

            if (!img.data)
            {
                cout << "ow" << endl;
                return -1;
            }
            ref = (ObjectPos*) cvAlloc(refcount * sizeof(*ref));
            for (i = 0; i < refcount; i++) {
                error = (fscanf(info, "%d %d %d %d", &x, &y, &width, &height)
                        != 4);
                if (error)
                    break;
                ref[i].x = 0.5F * width + x;
                ref[i].y = 0.5F * height + y;
                ref[i].width = sqrt(0.5F * (width * width + height * height));
                ref[i].found = 0;
                ref[i].neghbors = 0; //in the new cascade, where to get the neighbors?
            }

            vector<Rect> obj_detectados;
            Rect retang;
            std::vector<cv::Rect> rectangles_list;
            if (!error) {
                totaltime -= time(0);
//                cascade.detectMultiScale( img, obj_detectados, reject_levels, level_weights,scale_factor,1 ,0, cv::Size(),cv::Size(), true );
                cascade.detectMultiScale(img, obj_detectados, scale_factor, 6 , 0
                //|CV_HAAR_FIND_BIGGEST_OBJECT
                // |CV_HAAR_DO_ROUGH_SEARCH
                        //| CV_HAAR_SCALE_IMAGE
                		,cv::Size());

                totaltime += time(0);
                if (obj_detectados.size() == 0)
                {

                    cascade.detectMultiScale(img, obj_detectados, 1.05, 1 , 0

                    //|CV_HAAR_FIND_BIGGEST_OBJECT
                    // |CV_HAAR_DO_ROUGH_SEARCH
                            //| CV_HAAR_SCALE_IMAGE
                    		,cv::Size());
                }

                if (obj_detectados.size() == 0)

                {
                    cascade.detectMultiScale(img, obj_detectados, 1.03, 1 , 0
                    //|CV_HAAR_FIND_BIGGEST_OBJECT
                    // |CV_HAAR_DO_ROUGH_SEARCH
                            //| CV_HAAR_SCALE_IMAGE
                    		,cv::Size());
                }

                if (obj_detectados.size() == 0)
                {
                    detcount = 0;
               	}
                else
                {
                    detcount = obj_detectados.size();
                }



//                if (obj_detectados.size() > 1)
//                	for( unsigned int i = 0; i < obj_detectados.size(); i++ )
//                	{
//                		rectangles_list.push_back(obj_detectados[i]);
//                	}
//                	groupRectangles(rectangles_list,2,2.8);

                det = (detcount > 0) ?
                        ((ObjectPos*) cvAlloc(detcount * sizeof(*det))) : NULL;
                hits = missed = falseAlarms = 0;
		i =0;
		fprintf (detected_text_tags,"  <image>\n");
		fprintf (detected_text_tags,"    <imageName>tests/");
		fprintf (detected_text_tags,"%s", filename);
		fprintf (detected_text_tags,"</imageName>\n");
		fprintf (detected_text_tags,"    <taggedRectangles>\n");
		//fprintf (detected_text_tags,"%s \n", filename);
		for (vector<Rect>::const_iterator r = obj_detectados.begin(); r != obj_detectados.end(); r++, i++)
		{
			Point r1, r2;
			r1.x = (r->x);
			r1.y = (r->y);
			r2.x = (r->x + r->width);
			r2.y = (r->y + r->height);

			retang.x = r1.x;
			retang.y = r1.y;
			retang.width = abs(r2.x - r1.x);
			retang.height = abs(r2.y - r1.y);
			Mat roi = img(retang);
			Rect rectangle_info=get_rect_with_hough_line_transform(roi);
			Rect rectangle_updated_by_hough_line;
			rectangle_updated_by_hough_line.x=retang.x+rectangle_info.x;
			rectangle_updated_by_hough_line.y=retang.y+rectangle_info.y;
			rectangle_updated_by_hough_line.width=rectangle_info.width;
			rectangle_updated_by_hough_line.height=rectangle_info.height;



			std::vector<cv::Rect> detected_dashes_list=detect_dashes(rectangle_updated_by_hough_line,img);
			for (unsigned int p = 0; p < detected_dashes_list.size(); p++)
				{

					cv::Rect r = detected_dashes_list[p];
					cv::rectangle(img_copy, cv::Point(r.x,r.y),cv::Point(r.x+r.width,r.y+r.height), CV_RGB(191,62,255),1.5);
				}

			cv::Rect rectangle_updated_by_dashes_detection = restore_text_tag_by_detected_dashes(detected_dashes_list,rectangle_updated_by_hough_line,img);



		    //write the geometry infomation of detected text tags into xml file.

		    fprintf (detected_text_tags, "      <taggedRectangle x=\"");
		    fprintf (detected_text_tags, "%d", rectangle_updated_by_dashes_detection.x);
		    fprintf (detected_text_tags, "\" y=\"");
		    fprintf (detected_text_tags, "%d", rectangle_updated_by_dashes_detection.y);
		    fprintf (detected_text_tags, "\" width=\"");
		    fprintf (detected_text_tags, "%d", rectangle_updated_by_dashes_detection.width);
		    fprintf (detected_text_tags, "\" height=\"");
		    fprintf (detected_text_tags, "%d", rectangle_updated_by_dashes_detection.height);
		    fprintf (detected_text_tags, "\" modelType=\"1\"  />\n");
		    rectangle(img_copy, rectangle_updated_by_dashes_detection, cv::Scalar(0, 0, 255), 2, CV_AA);
		    rectangle(img_copy, retang, cv::Scalar(121, 150, 233), 1, CV_AA);
		    rectangle(img_copy, rectangle_updated_by_hough_line, cv::Scalar(0, 238, 0), 2, CV_AA);

		    //fprintf (detected_text_tags, "%d \n", retang.x);
		    //fprintf (detected_text_tags, "%d \n", retang.y);
		    //fprintf (detected_text_tags, "%d \n", retang.width);
		    //fprintf (detected_text_tags, "%d \n", retang.height);
            //rectangle(img, retang, Scalar(0, 0, 255), 1, CV_AA);

//		    fprintf (detected_text_tags, "      <taggedRectangle x=\"");
//		    fprintf (detected_text_tags, "%d", rectangle_updated_by_hough_line.x);
//		    fprintf (detected_text_tags, "\" y=\"");
//		    fprintf (detected_text_tags, "%d", rectangle_updated_by_hough_line.y);
//		    fprintf (detected_text_tags, "\" width=\"");
//		    fprintf (detected_text_tags, "%d", rectangle_updated_by_hough_line.width);
//		    fprintf (detected_text_tags, "\" height=\"");
//		    fprintf (detected_text_tags, "%d", rectangle_updated_by_hough_line.height);
//		    fprintf (detected_text_tags, "\" modelType=\"1\"  />\n");
//		    rectangle(img, rectangle_updated_by_hough_line, Scalar(0, 0, 255), 1, CV_AA);

//		    fprintf (detected_text_tags, "      <taggedRectangle x=\"");
//		    fprintf (detected_text_tags, "%d", retang.x);
//		    fprintf (detected_text_tags, "\" y=\"");
//		    fprintf (detected_text_tags, "%d", retang.y);
//		    fprintf (detected_text_tags, "\" width=\"");
//		    fprintf (detected_text_tags, "%d", retang.width);
//		    fprintf (detected_text_tags, "\" height=\"");
//		    fprintf (detected_text_tags, "%d", retang.height);
//		    fprintf (detected_text_tags, "\" modelType=\"1\"  />\n");

//		    if (rectangles_list.size()>0)
//		    {
//		    	//cout<<"rectangles_list size:"<< rectangles_list.size()<<endl;
//
//		    	rectangle( img, cv::Point(rectangles_list[1].x,rectangles_list[1].y), cv::Point(rectangles_list[1].x + rectangles_list[1].width,rectangles_list[1].y+rectangles_list[1].height), cv::Scalar( 255, 255, 0 ) , 4 , 8 , 0 );
//		    }
//		    printf(filename, "retangle.x: ", retang.x, "retangle.y: ", retang.y, "retangle.w: ", retang.width, "retangle.h: ", retang.height);
//
//		    if (saveDetected)
//			{
//        		std::stringstream ss;
////        		ss << reject_levels[i];
//        		std::string score = ss.str();
//        		ss << "";
////        		std::cout << score << '\n';
//        		std::stringstream ss2;
//        		ss2 << level_weights[i];
//        		std::string score2 = ss2.str();
//        		ss2 << "";

////        		cv::putText(img, score, cv::Point(retang.x, retang.y), 1 , 1, cv::Scalar(0,0,255));
//        		//cv::putText(img, score2, cv::Point(retang.x-10, retang.y-10), 1 , 1, cv::Scalar(0,0,0));
//			}

			det[i].x = 0.5F*r->width + r->x;
			det[i].y = 0.5F*r->height + r->y;
			det[i].width = sqrt(0.5F * (r->width * r->width + r->height * r->height));
			det[i].neghbors = 2; // i don't know if it will work...
			// det[i].neghbors = r.neighbors; --- how to do it in the new version??

			found = 0;
			for (j = 0; j < refcount; j++)
			{
				distance = sqrtf( (det[i].x - ref[j].x) * (det[i].x - ref[j].x) +
						(det[i].y - ref[j].y) * (det[i].y - ref[j].y) );
				//cout << distance << endl;
				if( (distance < ref[j].width * maxPosDiff) &&
						(det[i].width > ref[j].width / maxSizeDiff) &&
						(det[i].width < ref[j].width * maxSizeDiff) )
				{
					ref[j].found = 1;
					ref[j].neghbors = MAX( ref[j].neghbors, det[i].neghbors );
					found = 1;
				}
			}

				if (!found)
				{
					falseAlarms++;
					neg[MIN(det[i].neghbors, rocsize - 1)]++;
					//neg[MIN(0, rocsize - 1)]++;
				}
		}
                //imshow("teste", img);
                fprintf (detected_text_tags,"    </taggedRectangles>\n");
                fprintf (detected_text_tags,"  </image>\n");
                if (saveDetected)
                {
                    strcpy(detfilename, detname);
                    strcat(detfilename, filename);
                    strcpy(filename, detfilename);
                    imwrite(fullname, img_copy);
                    //cvvSaveImage(fullname, img);
                }

                for (j = 0; j < refcount; j++) {
                    if (ref[j].found)
                    {
                        hits++;
                        //pos[MIN(0, rocsize - 1)]++;
                        pos[MIN(ref[j].neghbors, rocsize - 1)]++;
                    }
                    else
                    {
                        missed++;
                    }
                }

                totalHits += hits;
                totalMissed += missed;
                totalFalseAlarms += falseAlarms;
                printf("|%32.64s|%6d|%6d|%6d|\n", filename, hits, missed,
                        falseAlarms);
                //printf("+--------------------------------+------+------+------+\n");
                fprintf(resultados, "|%32.64s|%6d|%6d|%6d|%10d|%10d|%10d|%10d|\n", filename, hits, missed, falseAlarms, retang.x,retang.y,retang.width,retang.height);
                //fprintf(resultados,
                //      "+--------------------------------+------+------+------+\n");
                fflush(stdout);
                if (det) {
                    cvFree( &det);
                    det = NULL;
                }
            } /* if( !error ) */

            //char c = (char) waitKey(10);
            //              if (c == 27)
            //                  exit(0);

            cvFree( &ref);
        }
        fclose(info);

        printf("|%32.32s|%6d|%6d|%6d|\n", "Total", totalHits, totalMissed,
                totalFalseAlarms);
        fprintf(resultados, "|%32.32s|%6d|%6d|%6d|\n", "Total", totalHits,
                totalMissed, totalFalseAlarms);
        printf("+===============================+======+======+======+==========+==========+==========+==========+\n");
        fprintf(resultados,
               "+===============================+======+======+======+==========+==========+==========+==========+\n");
        fprintf (detected_text_tags, "</tagset>\n");
        //printf("Number of stages: %d\n", nos);
        //printf("Number of weak classifiers: %d\n", numclassifiers[nos - 1]);
        printf("Total time: %f\n", totaltime);
        fprintf(resultados, "Total time: %f\n", totaltime);

        /* print ROC to stdout */
        for (i = rocsize - 1; i > 0; i--)
        {
            pos[i - 1] += pos[i];
            neg[i - 1] += neg[i];
        }
        //fprintf(stderr, "%d\n", nos);
        for (i = 0; i < rocsize; i++)
        {
            fprintf(stderr, "\t%d\t%d\t%f\t%f\n", pos[i], neg[i],
                    ((float) pos[i]) / (totalHits + totalMissed),
                    ((float) neg[i]) / (totalHits + totalMissed));
        }

        cvFree( &pos);
        cvFree( &neg);
    }

    return 0;
}
