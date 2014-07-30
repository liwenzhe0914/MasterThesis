// OpenCV includes
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Boost includes
#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"


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

//class MatchTemplate
//{
//public:
//	std::vector<std::string> filelist(std::string foldername);
//
//};



// main method

// Variables
// I/O

cv::Mat img;
cv::Mat templ;
cv::Mat result;
cv::Mat templ_pre;
cv::Mat templ_original;
std::string image_window = "Source Image";
std::string result_window = "Result window";
std::string path;
std::string letters_pre = "";

int match_method = 5;
double score=0.;
double score_pre = -1e10;

