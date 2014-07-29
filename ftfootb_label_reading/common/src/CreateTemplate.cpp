#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

int main() {
    vector<string> list1;
    vector<string> list2;
    vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    compression_params.push_back(9);
    string letters;
    string numbers;
    ifstream wordfile1("letters.dat");
    ifstream wordfile2("numbers.dat");
    while (wordfile1 >> letters) {
        list1.push_back(letters);
    }
    while (wordfile2 >> numbers) {
        list2.push_back(numbers);
    }

    for (int i=0; i<list1.size(); ++i) 
    {
	for (int j=0; j<list1.size(); ++j)
	{
		string letter_combnations = list1.at( i ) + list1.at( j );
		//cout<<combnations<<" ";
		Mat img(25, 40, CV_8UC3, Scalar(255,255,255));
		namedWindow( "Display window", WINDOW_AUTOSIZE );
		string namefont;
		namefont = "DejaVuSansMono";
		CvFont font = fontQt(namefont, 25, Scalar(0,0,0),CV_FONT_BOLD);
		addText( img, letter_combnations, Point(0,25), font);
		string filename = "/home/damon/MasterThesis/LettersTemplate/" + letter_combnations + ".png";
		imshow( "Display window", img );
      	imwrite(filename, img, compression_params);
	}
    }
    for (int m=0; m<list2.size(); ++m) 
    {
	for (int n=0; n<list2.size(); ++n)
	{
		string number_combnations = list2.at( m ) + list2.at( n );
		//cout<<combnations<<" ";
		Mat img(25, 40, CV_8UC3, Scalar(255,255,255));
		namedWindow( "Display window", WINDOW_AUTOSIZE );
		string namefont;
		namefont = "Lucida Console";
		CvFont font = fontQt(namefont, 25, Scalar(0,0,0),CV_FONT_BOLD);
		addText( img, number_combnations, Point(0,25), font);
		string filename = "/home/damon/MasterThesis/NumbersTemplate/" + number_combnations + ".png";
		imshow( "Display window", img );
      	imwrite(filename, img, compression_params);
	}
    }
    cout <<endl;
    return 0;
}

