#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

int main ( int argc, char** argv )
{
	int number=0;
	int loop = (argc-1)/4;
	std::cout<<"loop:"<<loop<<std::endl;
	for (int j = 0;j<loop;j++)
	{
		int first = 1+j*4;
		//std::cout<<"first:"<<first<<std::endl;
		int second = 2+j*4;
		//std::cout<<"second:"<<second<<std::endl;
		int third = 3+j*4;
		//std::cout<<"third:"<<third<<std::endl;
		int fourth = 4+j*4;
		//std::cout<<"fourth:"<<fourth<<std::endl;
		std::istringstream ss1(argv[first]);
		int x;
		if (!(ss1 >> x))
			std::cerr << "Invalid number " << argv[first] << '\n';
	//int x = argv[1];
		std::istringstream ss2(argv[second]);
		int y;
		if (!(ss2 >> y))
		std::cerr << "Invalid number " << argv[second] << '\n';
		std::istringstream ss3(argv[third]);
		int width;
		if (!(ss3 >> width))
		std::cerr << "Invalid number " << argv[third] << '\n';
		std::istringstream ss4(argv[fourth]);
		int height;
		if (!(ss4 >> height))
		std::cerr << "Invalid number " << argv[fourth] << '\n';
		int times = std::floor(width/height)+1;
		number = number + times;
		//std::cout<< "times"<<times<<std::endl;
		int width_changed = times*height;
	//std::cout<< "width_changed:"<<width_changed<<std::endl;
		double offset= (width_changed-width)/2;
		double x_changed = x - offset;
		for (int i = 0;i<times;i++)
		{
		double x_current = x_changed + i*height;
		std::cout<<x_current<<" "<<y<<" "<<height<<" "<<height<<" ";
		}
	}
	std::cout<<std::endl;
	std::cout<<number<<std::endl;




}
