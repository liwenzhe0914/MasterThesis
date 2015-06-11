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

	TextTagDetection(const std::string& path_data);

	inline void text_tag_detection_with_VJ(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list);

	void text_tag_detection_fine_detection(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list);

	int count_white_pixels_on_line(const cv::Mat& dst, const double r, const double cosine, const double sine, const bool vertical);

	inline float find_best_two_lines(const std::map<int,float>& lines_with_count_map);

	cv::Rect get_rect_with_hough_line_transform(const cv::Mat& src);

	// removes intersecting dashes
	void find_right_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& rect);

	void displayDashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Scalar& color, const cv::Mat& image);

	cv::Rect restore_text_tag_by_three_detected_dashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image);

	cv::Rect select_best_match_from_three_estimated_dashes(const cv::Rect& text_tag_l, const cv::Rect& text_tag_m, const cv::Rect& text_tag_r, const cv::Rect& text_tag, const cv::Mat& image);

	double template_matching_with_estimated_dashes(const cv::Mat& image);

	cv::Rect restore_tag_by_estimated_dashes(const cv::Point& estimated_dash_center, const cv::Rect& text_tag, const cv::Mat& image, std::vector<cv::Rect> detected_dashes_list);

	cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image);

	void unsharpMask(cv::Mat& im);

	double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

	double compare_detection_with_template(cv::Rect text_tag, cv::Mat img,std::string package_path);

	void detect_dashes(const cv::Rect& rect, const cv::Mat& image, std::vector<cv::Rect>& detected_dashes_list);

protected:

	cv::CascadeClassifier text_tags_cascade_;	///< Viola-Jones classifier model for text tag detection
	cv::Mat text_tag_template_image_;	///< template image of text tags
	cv::Size text_tag_template_target_size_;	///< desired size of templates
};
