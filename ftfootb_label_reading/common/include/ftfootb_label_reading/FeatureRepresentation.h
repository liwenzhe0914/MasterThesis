// OpenCV includes
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

// Boost includes
//#include <boost/filesystem.hpp>
//#include "boost/filesystem/operations.hpp"
//#include "boost/filesystem/fstream.hpp"


// Different includes
#include <time.h>
#include <dirent.h>
#include <cmath>
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <getopt.h>

//LBP
#include "ftfootb_label_reading/LBPandHistogram.h"


class FeatureReprenstation
{
public:

	cv::Rect remove_white_image_border(const cv::Mat& image, cv::Rect roi);

	cv::Mat get_feature_descriptor(const cv::Mat& img,int feature_number,int single_or_combination);

	int convertLettersToASCII(std::string letter,int single_or_combination);

	std::string convertASCIIToLetters(int number,int single_or_combination);

	std::vector<std::string> folder_list(std::string path);

	std::vector<std::string> load_folder_of_image(std::string path);

	void get_feature_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,
															int number_or_letter,int feature_number,int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels);

	void load_all_training_data_with_feature_descriptors(std::string training_path, int number_or_letter, int feature_type,
			int training_data_source, int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels);

	cv::Mat preprocess_text_tag(cv::Mat& tag_image, int feature_number, int single_or_combination);

	std::string read_text_tag_SVM(cv::SVM& numbers_svm, cv::SVM& letters_svm, cv::Mat& tag_image, int feature_number, int single_or_combination);
	std::string read_text_tag_KNN(cv::KNearest& numbers_knn,cv::KNearest& letters_knn,cv::Mat& testImg,int feature_number,int single_or_combination);
	cv::Mat wolf_thresholding(cv::Mat& img_gray);

	void help();

	void load_or_train_SVM_classifiers(cv::SVM& numbers_svm, cv::SVM& letters_svm, int training_data_source, int classifier,
			int feature_type, int single_or_combination, std::string path_data);

	void load_or_train_KNN_classifiers(cv::KNearest& numbers_knn, cv::KNearest& letters_knn, int training_data_source, int classifier,
			int feature_type, int single_or_combination, std::string path_data);

	cv::SVM numbers_svm_;
	cv::SVM letters_svm_;
	cv::KNearest numbers_knn_;
	cv::KNearest letters_knn_;

protected:

	inline std::string get_number_symbols_string(const int single_or_combination)
	{
		return (single_or_combination==1 ? "single" : "double");
	}

	inline std::string get_feature_type_string(const int feature_type)
	{
		return (feature_type==1 ? "hog" : (feature_type==2 ? "lbp" : "brief" ) );
	}
};
