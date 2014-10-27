// OpenCV includes
#include <opencv/cv.h>
#include <opencv/highgui.h>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/highgui/highgui.hpp>

// different includes
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>


std::string path(){
	char *path=NULL;
	size_t size = 0;
	path=getcwd(path,size);
	return path;
}
