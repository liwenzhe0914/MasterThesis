#include "opencv/cv.h"
#include "opencv/highgui.h"

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>

#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"
//#include "ftfootb_label_reading/labelBox.h"

using namespace std;
using namespace cv;

/// Global Variables
Mat img; Mat templ; Mat result; Mat templ_pre;Mat templ_original;
std::string image_window = "Source Image";
std::string result_window = "Result window";
int match_method = 5;
double score=0.;
//Point matchLoc_pre;
std::string letters_pre = "";

/**
 * @function main
 */
int main( int argc, char** argv )
{
	//matchLoc_pre.x = 0;
	//matchLoc_pre.y = 0;
	std::string letters;
	double start_time;
	double time_in_seconds;
	start_time = clock();
	if (argc < 2)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}

	std::vector<std::string> allImageNames;
	std::string arg(argv[2]);
	std::string imgpath = arg.substr(0, arg.find_last_of("/") + 1);
	//std::cout << "Image path:" << imgpath << std::endl;
	boost::filesystem::path input_path(argv[2]);
	//std::cout << "file:" << input_path << std::endl;
	img = imread( argv[1], 1 );
	//std::cout << "source imagename: " << argv[1] << std::endl;
	std::cout << "image size: " << img.cols<<"x"<<img.rows<< std::endl;
	
	/// read all the image files in input folder
	
	if (boost::filesystem::is_directory(input_path))
	{
		DIR *pDIR;
		struct dirent *entry;
		if ((pDIR = opendir(argv[2])))
		while ((entry = readdir(pDIR)))
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
				std::string s = argv[2];
				if (s.at(s.length() - 1) != '/')
					s.append("/");
				s.append(entry->d_name);
				allImageNames.push_back(s);
				//std::cout << "imagename: " << s << std::endl;
			}
		}
	}
	else
		allImageNames.push_back(argv[2]); //single image to be processed

	std::cout << "Images to be processed: " << allImageNames.size() << std::endl;

	std::map<std::string, cv::Mat> letter_templates;
	//letter_templates["AA"] = cv::imread

	int count = 0;
	double score_pre = -1e10;
	for (unsigned int i = 0; i < allImageNames.size(); i++)
	{	
		count++;
		templ_original = cv::imread(allImageNames[i],CV_LOAD_IMAGE_COLOR); // originalImage like it used to be, never modify
		Size size( (img.rows-1)*templ_original.cols/templ_original.rows,img.rows-1);
		std::cout << "size:" << size << std::endl;
		resize(templ_original,templ,size);//resize image
		//std::cout << "current template imagename: " << allImageNames[i]<< std::endl;
		/// get image name without extension and path
		int lastindex = allImageNames[i].find_last_of("."); 
		int lastindex2 = allImageNames[i].find_last_of("/"); 
		string rawname = allImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
		std::cout << "Current template:" << rawname << std::endl;		
		letters=rawname;
		/// Create windows
		namedWindow( image_window, CV_WINDOW_AUTOSIZE );
		namedWindow( result_window, CV_WINDOW_AUTOSIZE );

		  /// Source image to display
		Mat img_display;
		img.copyTo( img_display );
  
		/// Create the result matrix
		int result_cols =  img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;
		//std::cout << "img.cols:" << img.cols <<std::endl;
		//std::cout << "img.rows:" << img.rows <<std::endl;
		//std::cout << "templ.cols:" << templ.cols <<std::endl;
		//std::cout << "templ.rows:" << templ.rows <<std::endl;
		result.create( result_cols, result_rows, CV_32FC1 );
  
		/// Do the Matching and Normalize
		cv::Mat img_roi = img(cv::Rect(0,0,0.25*img.cols,img.rows));
		matchTemplate( img_roi, templ, result, match_method );
		std::cout << "match_method:" << match_method <<std::endl;
		//cv::Mat result2;
		//normalize( result, result2, 0, 1, NORM_MINMAX, -1, Mat() );
		//cv::imshow("result2",result2);
		//waitKey(0);
		/// Localizing the best match with minMaxLoc
		double minVal; double maxVal; Point minLoc; Point maxLoc;
		Point matchLoc;
		minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

		//std::cout<<"maxVal: "<<maxVal<<endl;
		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
			{ matchLoc = minLoc; 
			score=minVal;}
		else  
			{ matchLoc = maxLoc; 
			score= maxVal;}
		//std::cout << "matchLoc:" << matchLoc <<std::endl;
		//matchLoc_pre = matchLoc;
		//std::cout << "matchLoc_pre: " << matchLoc_pre<< std::endl;
		/// comparision bewteen template and matched part
		//cv::Rect box(matchLoc.x,matchLoc.y,templ.cols,templ.rows);
		//cv::Mat matchedPart = img(box);
		//imshow( "matched part ", matchedPart);
		//imshow( "templ", templ);
  
		/// score the matching
  
		//for (int m=0; m<templ.cols;m++)
		//	{
		//	for (int n=0; n < templ.rows;n++)
		//		{
		//			if (abs(templ.at<int>(m,n) - matchedPart.at<int>(m,n)) < 10)
		//				score_pre++;
		//		}
		//	}
		
		//score_pre = score_pre/(templ.cols*templ.rows);
		//std::cout << "score_previous: " << score_pre<< std::endl;
		std::cout << "current score: " << score<< std::endl;
		/// update the score when new score is higher
		if (score_pre > score)	
			{score = score_pre;
			letters = letters_pre;
			//matchLoc_pre = matchLoc;
			templ.copyTo( templ_pre );}
		else
			{
			//matchLoc = matchLoc_pre;
			templ_pre.copyTo( templ );}
		
		std::cout << "Best score so far: " << score<< std::endl;
		std::cout << "matchLoc: " << matchLoc<< std::endl;
		if (matchLoc.x<5)
		{score_pre = score;
		letters_pre=letters;}
		/// Show me what you got
		//std::cout << "matchLoc: " << matchLoc<< std::endl;
		rectangle( img_display, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 ); 
		rectangle( result, matchLoc, Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), Scalar::all(0), 2, 8, 0 ); 
		imshow( image_window, img_display );
		imshow( result_window, result );
		cout<<"current best match so far: "<<letters<<endl;
		//waitKey(0);
		cout<<"-----------------------------------"<<endl;
	}
  time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
  std::cout << "Detected letters/numbers are :"<<  letters << std::endl;
  std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
  //cout<< "count: "<< count <<endl;
  waitKey(0);
  return 0;
}
