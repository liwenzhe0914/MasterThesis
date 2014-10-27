// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

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

std::vector<std::string> list1;
std::vector<std::string> list2;
std::string letters;
std::string numbers;
std::ifstream wordfile1;
std::ifstream wordfile2;
std::string letter_combnations;
std::string number_combnations;
std::string namefont;
std::string filename1;
std::string filename2;
//std::string current_path;
