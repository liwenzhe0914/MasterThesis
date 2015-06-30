#include <iostream>
#include <fstream>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//using namespace std;

int main ()
{
	unsigned x, y, width, height,i;
	std::string newFileName = "/home/damon/Desktop/GT_SWT.xml";
	std::ofstream newFile;
	newFile.open(newFileName.c_str());
	newFile << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"<<endl;
	newFile << "<tagset>"<< std::endl;
	std::string STRING;
	std::ifstream infile;
	infile.open ("/home/damon/git/imageclipper/info.dat");
    while(!infile.eof()) // To get you all the lines.
    {
    	getline(infile,STRING); // Saves the line in STRING.
    	std::cout<<STRING<<std::endl; // Prints our STRING.
		std::string remainingString = STRING;
		std::string imagename = remainingString.substr(0, remainingString.find_first_of(' ')).c_str();
		newFile <<"  <image>"<<std::endl<<"    <imageName>"<<"VJ_test_warehouse_dataset/"<<imagename<<"</imageName>"<<endl
				<<"    <taggedRectangles>"<<std::endl;

		remainingString = remainingString.substr(remainingString.find_first_of(' ') + 1, remainingString.length()
				                - remainingString.find_first_of(' '));
		i = atoi( remainingString.substr(0, 1).c_str() );
		remainingString = remainingString.substr(1 + 1, remainingString.length()
						                - 1);
		for (unsigned int j=0; j<i; j++)
		{
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
		}
		newFile<<"    </taggedRectangles>"<<endl<<"  </image>"<<endl;
    }
	infile.close();
}
