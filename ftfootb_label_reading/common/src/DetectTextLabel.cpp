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
	text_tags_cascade.load( "/home/damon/git/opencv-haar-classifier-training/text_tag_classifiers_3/cascade.xml" );
	//face_cascade.load( "/home/rmb-lw/git/opencv-haar-classifier-training/haarcascade_3/haarcascade.xml" );

	// Detect faces
	std::vector<cv::Rect> texts;
	std::vector<cv::Rect> texts2;
	std::vector<cv::Rect> rectangles_list;
	cv::Rect group_rectangle;
	std::vector<int> reject_levels;
	std::vector<double> level_weights;
	//text_tags_cascade.detectMultiScale( image, texts,reject_levels, level_weights, 1.05 , 1,0, cv::Size(35, 9),cv::Size(710, 180), true);
	//text_tags_cascade.detectMultiScale( image, texts_2, 1.04 , 4,0, cv::Size(35, 9),cv::Size(710, 180));
	text_tags_cascade.detectMultiScale( image, texts, reject_levels, level_weights,1.05,1,0, cv::Size(35, 9),cv::Size(), true );
	text_tags_cascade.detectMultiScale( image, texts2,1.03 ,2,0, cv::Size(35, 9),cv::Size());
	cv::Mat img_color = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);

	//std::cout << reject_levels.at(1) << '\n';
	// Draw circles on the detected faces
	for( unsigned int i = 0; i < texts.size(); i++ )
	{
rectangle( img_color, cv::Point(texts[i].x,texts[i].y), cv::Point(texts[i].x + texts[i].width,texts[i].y+texts[i].height), cv::Scalar( 255, 0, 0 ) , 1 , 8 , 0 );

	}
	for( unsigned int i = 0; i < texts2.size(); i++ )
	{

/*		rectangles_list.push_back(texts[i]);
		std::stringstream ss;
		ss << reject_levels[i];
		std::string score = ss.str();
		ss << "";
		std::cout << score << '\n';
		std::stringstream ss2;
		ss2 << level_weights[i];
		std::string score2 = ss2.str();
		ss2 << "";
		std::cout << score << '\n';
		std::cout<<rectangles_list.size()<<std::endl;
*/
		//rectangle( img_color, cv::Point(texts[i].x,texts[i].y), cv::Point(texts[i].x + texts[i].width,texts[i].y+texts[i].height), cv::Scalar( 255, 0, 0 ) , 1 , 8 , 0 );
std::cout << "im here" << '\n';
//rectangle( img_color, cv::Point(texts2[i].x,texts2[i].y), cv::Point(texts2[i].x + texts2[i].width,texts2[i].y+texts2[i].height), cv::Scalar( 0, 0, 255 ) , 2 , 8 , 0 );
		//rectangle(img_color, texts2[i], cv::Scalar(255,255,255), 3 , 8 , 0);
		//cv::putText(img_color, score, cv::Point(texts[i].x, texts[i].y), 1 , 1, cv::Scalar(0,0,255));
		//cv::putText(img_color, score2, cv::Point(texts[i].x-10, texts[i].y-10), 1 , 1, cv::Scalar(0,0,0));


	}
/*	groupRectangles(rectangles_list,1,0.8);
	std::cout<<rectangles_list.size()<<std::endl;
	if (rectangles_list.size()>0)
		rectangle( img_color, cv::Point(rectangles_list[1].x,rectangles_list[1].y), cv::Point(rectangles_list[1].x + rectangles_list[1].width,rectangles_list[1].y+rectangles_list[1].height), cv::Scalar( 255, 255, 0 ) , 2 , 8 , 0 );
	else
		for( unsigned int i = 0; i < texts.size(); i++ )
			rectangle( img_color, cv::Point(texts[i].x,texts[i].y), cv::Point(texts[i].x + texts[i].width,texts[i].y+texts[i].height), cv::Scalar( 255, 255, 0) , 3 , 8 , 0 );
*/
	imshow( "Detected Text Tags", img_color );
	time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
	cv::waitKey(0);
	return 0;
}
