#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <vector>
#include <iostream>
#include "lbp.hpp"
#include <set>
#include "histogram.hpp"

using namespace cv;
using namespace std;

int main()
{
	//variables
	char FullFileName[100];
	//char FirstFileName[100]="/home/damon/Desktop/test_XC_";
	char FirstFileName[100]="/home/rmb-om/Desktop/text_portion_";
	//char SaveHogDesFileName[100] = "Positive.xml";
	int FileNum=1;


	for(int i=0; i< FileNum; ++i)
	{
		sprintf(FullFileName, "%s%d.png", FirstFileName, i+1);
		printf("%s\n", FullFileName);

		//read image file
		cv::Mat img, img_gray,lbp_image,hist;
		img = imread(FullFileName);
		resize(img, img, Size(64,64));
		cvtColor(img, img_gray, CV_RGB2GRAY);

		lbp::OLBP(img_gray,lbp_image);
		cout<<lbp_image.cols<<"x"<<lbp_image.rows<<" "<<lbp_image.type()<<endl;
		normalize(lbp_image, lbp_image, 0, 255, NORM_MINMAX, CV_8UC1);
		Mat lbp_image_resized;
		resize(lbp_image, lbp_image_resized, Size(lbp_image.cols*4.0, lbp_image.rows*4.0));
		cout<<"lbp_image_resized: "<<lbp_image_resized<<endl;

		imshow("lbp_image_resized",lbp_image);
		imshow("0_",img_gray);
		waitKey(0);
		hist=lbp::spatial_histogram(lbp_image, 59,8, 8,true);
		cout<<"hist: "<<hist<<endl;
/*		namedWindow( "0_1", CV_WINDOW_AUTOSIZE );
		imshow("0_1",hist);*/
		waitKey(0);
	}
}
