// OpenCV includes
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
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


struct TagDetectionData
{
	cv::RotatedRect min_area_rect_;
	std::vector<cv::Point2f> corners_;

	TagDetectionData(const cv::RotatedRect& min_area_rect, const std::vector<cv::Point>& corners)
	{
		min_area_rect_ = min_area_rect;

		std::vector<cv::Point2f> corners2f(corners.size());
		for (size_t i=0; i<corners.size(); ++i)
			corners2f[i] = cv::Point2f(corners[i].x, corners[i].y);
		cv::Point2f rect_points[4];
		min_area_rect_.points(rect_points);
		for (int i=0; i<4; ++i)
			corners_.push_back(get_nearest_point(rect_points[i], corners2f));
	}

	TagDetectionData(const cv::RotatedRect& min_area_rect, const std::vector<cv::Point2f>& corners)
	{
		min_area_rect_ = min_area_rect;

		cv::Point2f rect_points[4];
		min_area_rect_.points(rect_points);
		for (int i=0; i<4; ++i)
			corners_.push_back(get_nearest_point(rect_points[i], corners));
	}

	cv::Point2f get_nearest_point(const cv::Point2f& reference_point, const std::vector<cv::Point2f>& target_points)
	{
		double min_dist = 1e20;
		cv::Point2f min_point(0,0);
		for (size_t i=0; i<target_points.size(); ++i)
		{
			double dist = (reference_point.x-target_points[i].x)*(reference_point.x-target_points[i].x) + (reference_point.y-target_points[i].y)*(reference_point.y-target_points[i].y);
			if (dist < min_dist)
			{
				min_dist = dist;
				min_point = target_points[i];
			}
		}

		return min_point;
	}
};

class TextTagDetection
{
public:

	TextTagDetection(const std::string& path_data);

	inline void text_tag_detection_with_VJ(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list);

	void text_tag_detection_fine_detection_vj(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list);
	void text_tag_detection_fine_detection_rectangle_detection(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list, std::vector<TagDetectionData>& detections_r);

	int count_white_pixels_on_line(const cv::Mat& dst, const double r, const double cosine, const double sine, const bool vertical);

	inline float find_best_two_lines(const std::map<int,float>& lines_with_count_map);

	cv::Rect get_rect_with_hough_line_transform(const cv::Mat& src);

	// removes intersecting dashes
	void find_right_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& rect);

	void displayDashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Scalar& color, const cv::Mat& image);

	cv::Rect restore_text_tag_by_three_detected_dashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image);

	cv::Rect select_best_match_from_three_estimated_dashes(const cv::Rect& text_tag_l, const cv::Rect& text_tag_m, const cv::Rect& text_tag_r, const cv::Rect& text_tag, const cv::Mat& image);

	double compare_detection_with_template(const cv::Mat& image);

	cv::Rect restore_tag_by_estimated_dashes(const cv::Point& estimated_dash_center, const cv::Rect& text_tag, const cv::Mat& image, std::vector<cv::Rect> detected_dashes_list);

	cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image);

	void unsharpMask(cv::Mat& im);

	double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

//	double compare_detection_with_template(cv::Rect text_tag, cv::Mat img,std::string package_path);

	void detect_dashes(const cv::Rect& rect, const cv::Mat& image, std::vector<cv::Rect>& detected_dashes_list);

	void cut_out_rotated_rectangle(const cv::RotatedRect& rotated_rect, const cv::Mat& image, cv::Mat& rotated_image);

	void remove_projection(const cv::RotatedRect& rotated_rect, const cv::Mat& image, cv::Mat& rectified_image);
	void remove_projection(const TagDetectionData& detection, const cv::Mat& image, cv::Mat& rectified_image);

	void refine_detection(TagDetectionData& detection, const cv::Mat& image);

	cv::Point2f line_intersection(const cv::Vec4f& line1, const cv::Vec4f& line2);

	// mode: 0=left line, 1=upper line, 2=right line, 3=lower line
	cv::Vec4f fit_tag_lines(const cv::Mat& area_image, const int mode);

	void fit_line(const std::vector<cv::Point2f>& points, cv::Vec4f& line, double inlier_ratio, double success_probability, double max_inlier_distance, bool draw_from_both_halves_of_point_set=false);

	void detect_tag_by_frame(const cv::Mat& image_grayscale, std::vector<cv::Rect>& detections, std::vector<TagDetectionData>& detections_r);

	void correct_rotated_rect_rotation(cv::RotatedRect& rotated_rect);

protected:

	cv::CascadeClassifier text_tags_cascade_;	///< Viola-Jones classifier model for text tag detection
	cv::Mat text_tag_template_image_;	///< template image of text tags
	cv::Size text_tag_template_target_size_;	///< desired size of templates
};
