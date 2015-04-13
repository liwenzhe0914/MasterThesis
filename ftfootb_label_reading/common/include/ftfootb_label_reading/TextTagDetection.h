// OpenCV includes
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


// Different includes
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

class TextTagDetection
{
public:
	std::vector<cv::Rect> text_tag_detection_with_VJ(cv::Mat img);

	std::vector<cv::Rect> text_tag_detection_fine_detection(cv::Mat img);

	int count_white_pixels(cv::Mat dst,cv::Point pt1,cv::Point pt2, bool vertical);

	float find_best_two_lines(std::map<float,float> lines_with_count_map);

	cv::Rect get_rect_with_hough_line_transform(cv::Mat src);

	std::vector<cv::Rect> find_right_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Mat img,cv::Rect rect);

	cv::Rect restore_text_tag_by_three_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img);

	cv::Rect select_best_match_from_three_estimated_dashes(cv::Rect text_tag_l,cv::Rect text_tag_m,cv::Rect text_tag_r,cv::Rect text_tag,cv::Mat img);

	cv::Rect restore_tag_by_estimated_dashes(cv::Point estimated_dash_center,cv::Rect text_tag,cv::Mat img,std::vector<cv::Rect> detected_dashes_list);

	cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img);

	void unsharpMask(cv::Mat& im);

	double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

	std::vector<cv::Rect>detect_dashes (cv::Rect rect,cv::Mat img);
};
