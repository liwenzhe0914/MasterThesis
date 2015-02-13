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

int main (int argc, char** argv)
{
	std::string folder=argv[1];
	int lastindex_tmp = folder.find_last_of("/");
	std::string foldername_tmp = folder.substr(0,lastindex_tmp);
	int lastindex = foldername_tmp.find_last_of("/");
	std::string foldername = foldername_tmp.substr(lastindex+1);
	std::string path = foldername_tmp.substr(0,lastindex);

	//std::cout<<foldername<<std::endl;
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
		char *cstr2,*cstr1,*cstr3;
		for (unsigned int i = 0; i < ImageNames.size(); i++)
			{
				std::cout<<"loop:"<<i<<std::endl;
//				std::cout<<"ImageNames:"<<ImageNames[i]<<std::endl;
				std::stringstream ss;
				ss<<path<<"/"<<foldername<<"/"<<foldername<<"_"<<i+1<<".png";
				std::string new_name = ss.str();
				ss.str("");
				cstr1 = new char[new_name.length() + 1];
				strcpy(cstr1, new_name.c_str());
				std::cout<<"new name char:"<<cstr1<<std::endl;
				int lastindex = ImageNames[i].find_last_of("/");
				std::string raw_image_name = ImageNames[i].substr(lastindex+1);
//				std::cout<<"old name000:"<<ImageNames[i]<<std::endl;
//				std::cout<<"old name:"<<raw_image_name<<std::endl;
				cstr2 = new char[raw_image_name.length() + 1];
				strcpy(cstr2, raw_image_name.c_str());
				std::cout<<"old name char:"<<cstr2<<std::endl;
				cstr3 = new char[ImageNames[i].length() + 1];
				strcpy(cstr3, ImageNames[i].c_str());
				std::rename(cstr3,cstr1);
				std::cout<<"ImageNames char:"<<cstr3<<std::endl;
			}
		delete [] cstr1;
		delete [] cstr2;
		delete [] cstr3;
}
