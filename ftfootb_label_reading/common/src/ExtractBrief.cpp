#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include <vector>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
 //variables
 char FullFileName[100];
 //char FirstFileName[100]="/home/damon/Desktop/test_XC_";
 char FirstFileName[100]="/home/damon/git/opencv-haar-classifier-training/test_XC_";
 //char SaveHogDesFileName[100] = "Positive.xml";
 int FileNum=1;
 int class_number=1;
 vector<KeyPoint> keyPoints;
 vector<KeyPoint> keyPoints_extend;
 vector<KeyPoint> keyPoints2;
 Mat descriptorsValues;
 Mat descriptorsValues2;
 Mat descriptorsValues3;
 DenseFeatureDetector detector(1.0f, 1, 0.5f, 50);
 StarFeatureDetector detector2(11,1,100,80,1);
 BriefDescriptorExtractor extractor(32);

 std::ofstream out_file;
 out_file.open("BRIEFFeatures.txt");
 out_file<<class_number<<" ";
 for(int i=0; i< FileNum; ++i)
 {
	 sprintf(FullFileName, "%s%d.jpg", FirstFileName, i+1);
	 printf("%s\n", FullFileName);

  //read image file
	 Mat img, img_gray;
	 img = imread(FullFileName);
	 resize(img, img, Size(57,57));
	 //img_gray = imread(FullFileName,CV_LOAD_IMAGE_GRAYSCALE);

  //gray
	 cvtColor(img, img_gray, CV_RGB2GRAY);
  // detect keypoints
	 detector.detect(img_gray,keyPoints);
	 detector2.detect(img_gray,keyPoints2);
	 cout << "found " << keyPoints.size() << " keypoints in " << FullFileName << endl;
	 cout << "found " << keyPoints[1].angle << " angle"<< endl;
	 cout << "found " << keyPoints[1].response << " response"<< endl;
	 cout << "found " << keyPoints[1].octave << " octave"<< endl;
	 cout << "found " << keyPoints[1].class_id << " class_id"<< endl;
	 cout << "found " << keyPoints[2].size << " size"<< endl;
	 cout << "found " << keyPoints2.size() << "STAR keypoints in " << FullFileName << endl;
	 Mat img_kp;
	 Mat img_kp2;
	 Mat img_kp3;

//	 for (unsigned i = 0;i<keyPoints.size();++i)
//	 {
//		 1;
//		 cout<<i+1<<" keypoint:"<<keyPoints.at(i).pt<<endl;
//	 }
//
//	 for (unsigned i = 0;i<keyPoints2.size();++i)
//	 {
//		 1;
//		 //cout<<i+1<<" keypoint2:"<<keyPoints2.at(i).pt<<endl;
//	 }
	 //extract feature
	 vector<KeyPoint> keyPoints3;
	 KeyPoint pt3 (float(img_gray.cols/2),float(img_gray.rows/2),1,-1, 0, 0, -1);
	 keyPoints3.push_back(pt3);
	 drawKeypoints(img_gray,keyPoints,img_kp,Scalar::all(-1));
	 drawKeypoints(img_gray,keyPoints2,img_kp2,Scalar(0,0,255));
	 drawKeypoints(img_gray,keyPoints3,img_kp3,Scalar(0,0,255));
	 namedWindow( "Dense keypoints", CV_WINDOW_AUTOSIZE );
	 imshow("Dense keypoints",img_kp);
	 namedWindow( "STAR keypoints2", CV_WINDOW_AUTOSIZE );
	 imshow("STAR keypoints2",img_kp2);
	 namedWindow( "center", CV_WINDOW_AUTOSIZE );
	 imshow("center",img_kp3);
	 waitKey(0);
	 cout << "!!!found " << keyPoints.size() << " keypoints in " << FullFileName << endl;
	 cout << "!!!found " << keyPoints2.size() << "STAR keypoints in " << FullFileName << endl;
	 cout << "!!!found " << keyPoints3.size() << "center keypoints in " << FullFileName << endl;
	 extractor.compute(img_gray,keyPoints,descriptorsValues);
	 extractor.compute(img_gray,keyPoints2,descriptorsValues2);
	 extractor.compute(img_gray,keyPoints3,descriptorsValues3);
	 cout << "!!!found " << keyPoints.size() << " keypoints in " << FullFileName << endl;
	 cout << "!!!found " << keyPoints2.size() << "STAR keypoints in " << FullFileName << endl;
	 cout << "!!!found " << keyPoints3.size() << "center keypoints in " << FullFileName << endl;
	 cout<<"im here"<<endl;
	 int rows =descriptorsValues.rows;
	 int cols =descriptorsValues.cols;
	 cout<<"size:"<<rows<<"x"<<cols<<endl;
	 int rows2 =descriptorsValues2.rows;
	 int cols2 =descriptorsValues2.cols;
	 cout<<"size:"<<rows2<<"x"<<cols2<<endl;
	 int rows3 =descriptorsValues3.rows;
	 int cols3 =descriptorsValues3.cols;
	 cout<<"size:"<<rows3<<"x"<<cols3<<endl;
//	 namedWindow( "descriptorsValues", CV_WINDOW_AUTOSIZE );
//	 imshow("descriptorsValues",descriptorsValues);
//	 namedWindow( "descriptorsValues2", CV_WINDOW_AUTOSIZE );
//	 imshow("descriptorsValues2",descriptorsValues2);
//	 cout << "descriptorsValues2 = "<< endl << " "  << descriptorsValues2 << endl << endl;
	cout<<descriptorsValues3<<endl;

	 Mat img_kp_1;
	 Mat img_kp_2;
	 Mat img_kp_3;
	 drawKeypoints(img_gray,keyPoints,img_kp_1,Scalar::all(-1));
	 drawKeypoints(img_gray,keyPoints2,img_kp_2,Scalar::all(-1));
	 drawKeypoints(img_gray,keyPoints3,img_kp_3,Scalar::all(-1));
	 namedWindow( "FAST keypoints_", CV_WINDOW_AUTOSIZE );
	 imshow("FAST keypoints_",img_kp_1);

	 namedWindow( "STAR keypoints2_", CV_WINDOW_AUTOSIZE );
	 imshow("STAR keypoints2_",img_kp_2);

	 namedWindow( "center_", CV_WINDOW_AUTOSIZE );
	 imshow("center_",img_kp_3);
	 waitKey(0);
 }
  // output descriptor values
/*	 for(unsigned int j=1; j<= descriptorsValues.size(); ++j)
	{
		 if (descriptorsValues[j-1] !=0)
		 {
//			 cout<<j<<":"<<descriptorsValues[j-1]<<" "<<" ";
	 	 	 out_file<<j<<":"<<descriptorsValues[j-1]<<" ";
		 }
	}*/
 /* //printf("descriptor number =%d\n", descriptorsValues.size() );
	 v_descriptorsValues.push_back( descriptorsValues );
	 v_locations.push_back( locations );
  //show image
	 //imshow("origin", img);

	 Mat visual_image = get_hogdescriptor_visual_image(img,descriptorsValues,Size(64,64),Size(8,8),4,4.0);
	 imshow("origin", visual_image);


	 Mat img_bw;
	 threshold(img_gray, img_bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	 vector< float> descriptorsValues_bw;
	 d.compute( img_bw, descriptorsValues_bw, Size(0,0), Size(0,0), locations);
	 Mat visual_image_bw = get_hogdescriptor_visual_image(img,descriptorsValues_bw,Size(64,64),Size(8,8),4,4.0);
	 imshow("otsu",img_bw);
	 imshow("otsu with HOG", visual_image_bw);*/
	 waitKey(0);
 }
