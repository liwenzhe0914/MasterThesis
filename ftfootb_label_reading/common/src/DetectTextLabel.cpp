#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

int main( int argc, char** argv )
{
	double start_time;
	double time_in_seconds;
	start_time = clock();
	cv::Mat image;
	image = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
	cv::equalizeHist(image, image);
	cv::namedWindow( "result", 1 );
	//imshow( "result", image );

	// Load Text cascade (.xml file)
	cv::CascadeClassifier text_tags_cascade;
	text_tags_cascade.load( "/home/rmb-lw/git/opencv-haar-classifier-training/text_tag_classifiers_3/cascade.xml" );
	//face_cascade.load( "/home/rmb-lw/git/opencv-haar-classifier-training/haarcascade_3/haarcascade.xml" );

	// Detect faces
	std::vector<cv::Rect> texts;
	std::vector<int> reject_levels;
	std::vector<double> level_weights;
	//text_tags_cascade.detectMultiScale( image, texts,reject_levels, level_weights, 1.05 , 1,0, cv::Size(35, 9),cv::Size(710, 180), true);
	text_tags_cascade.detectMultiScale( image, texts, 1.05 , 1,0, cv::Size(35, 9),cv::Size(710, 180));
	//text_tags_cascade.detectMultiScale( image, texts, reject_levels, level_weights,1.2, 1,0, cv::Size(),cv::Size() );
	cv::Mat img_color = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);
	//std::cout << reject_levels[1] << '\n';
	// Draw circles on the detected faces
	for( int i = 0; i < texts.size(); i++ )
	{
//		std::ostringstream convert;
//		convert << reject_levels[i];
//		std::string reject_levels = convert.str();
//		std::cout << reject_levels << '\n';
		rectangle( img_color, cv::Point(texts[i].x,texts[i].y), cv::Point(texts[i].x + texts[i].width,texts[i].y+texts[i].height), cv::Scalar( 255, 0, 255 ) , 3 , 8 , 0 );
		//rectangle(img_color, texts[i], cv::Scalar(255,0,0), 1);
		//cv::putText(img_color,reject_levels, cv::Point(texts[i].x, texts[i].y), 1 , 1, cv::Scalar(0,0,255));

	}

	imshow( "Detected Text Tags", img_color );
	time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
	cv::waitKey(0);
	return 0;
}
