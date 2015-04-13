// OpenCV includes
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

// Boost includes
#include <boost/filesystem.hpp>
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/fstream.hpp"


// Different includes
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>

//LBP
#include "ftfootb_label_reading/LBPandHistogram.h"


class FeatureReprenstation
{
public:

		std::vector<float> get_HOG_descriptor(cv::Mat img);

		cv::Mat get_LBP_descriptor(cv::Mat img);

		std::vector<float> get_BRIEF_descriptor(cv::Mat img);

		int convertLettersToASCII(std::string letter);

		std::string convertASCIIToLetters(int number);

		std::vector<std::string> folder_list(std::string path);

		std::vector<std::string> load_folder_of_image(std::string path);

		cv::Mat FeatureReprenstation::get_feature_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,int number_or_letter,int feature_number);

		cv::Mat load_all_training_data_with_feature_descriptors(std::string training_path,int number_or_letter,int feature_number,int load);

		std::string read_text_tag(cv::Mat image,int load);

};
