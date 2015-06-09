// OpenCV includes
#include "opencv2/opencv.hpp"
#include "opencv/cv.h"
#include "opencv/highgui.h"

// Boost includes
//#include <boost/filesystem.hpp>
//#include "boost/filesystem/operations.hpp"
//#include "boost/filesystem/fstream.hpp"


//ros includes
#include <ros/ros.h>
#include <ros/package.h>
#include <set>

// Different includes
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>

class MatchTemplate
{
public:

	typedef std::map<std::string, cv::Mat> TokenTemplates;
	typedef TokenTemplates::iterator TokenTemplatesIterator;

	MatchTemplate(const std::string& templates_storage_path);

	void load_templates(const std::string& templates_storage_path, TokenTemplates& token_templates);

	void resize_templates(TokenTemplates& token_templates);

	// divide a tag into single regions (letter pairs, number pairs) and read from them using template matching
	void read_tag(const cv::Mat& tag_image, std::string& tag_label,int match_method);

	// matches a set of templates to image
	// it is assumed that there is only one valid match within image
	// low matching scores are best, matching_scores.begin() has the lowest score
	void match_token_templates(const cv::Mat& image, const TokenTemplates& token_templates,
								std::multimap<double, std::string>& matching_scores,int match_method);

private:

	TokenTemplates letter_pair_templates;	// first: name of letter combination (e.g. "AA", "GF"), second: images of letter templates
	TokenTemplates number_pair_templates;	// first: name of number combination (e.g. "73", "34"), second: images of number templates

	// parameters
	double standard_tag_image_height_;


};
