#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int main ()
{
	unsigned x, y, width, height,i;
	std::string newFileName = "/home/damon/git/opencv-haar-classifier-training/test_hough/hough_line_detection.xml";
	std::ofstream newFile;
	newFile.open(newFileName.c_str());
	newFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	newFile << "<tagset>"<< std::endl;
	string STRING;
	ifstream infile;
	infile.open ("/home/damon/git/opencv-haar-classifier-training/test_hough/hough_line_detection.txt");
    while(!infile.eof()) // To get you all the lines.
    {
    	getline(infile,STRING); // Saves the line in STRING.
		cout<<STRING<<endl; // Prints our STRING.
		std::string remainingString = STRING;
		string imagename = remainingString.substr(0, remainingString.find_first_of(' ')).c_str();
		newFile <<"  <image>"<<endl<<"    <imageName>"<<"test_hough/"<<imagename<<"</imageName>"<<endl
				<<"    <taggedRectangles>"<<endl;

		remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
				                - remainingString.find_first_of(' '));
			x = atoi(remainingString.substr(0, remainingString.find_first_of(' ')).c_str());
			remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
			                - remainingString.find_first_of(' '));
			y = atoi(remainingString.substr(0, remainingString.find_first_of(' ')).c_str());
			remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
			                - remainingString.find_first_of(' '));
			width = atoi(remainingString.substr(0, remainingString.find_first_of(' ')).c_str());
			remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
			                - remainingString.find_first_of(' '));
			height = atoi(remainingString.substr(0, remainingString.find_first_of(' ')).c_str());
			remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
			                - remainingString.find_first_of(' '));

		newFile <<"      <taggedRectangle x=\"" << x << "\" y=\"" << y << "\" width=\"" << width << "\" height=\""
		                << height << "\" modelType=\"1\" />" << std::endl;
		newFile<<"    </taggedRectangles>"<<endl<<"  </image>"<<endl;
    }
	infile.close();
}
