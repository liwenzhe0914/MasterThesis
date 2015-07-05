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


class FeatureRepresentation
{
public:

	cv::Rect remove_white_image_border(const cv::Mat& image, cv::Rect roi);

	cv::Mat get_feature_descriptor(const cv::Mat& image, int feature_type, int single_or_combination);

	int convertLettersToASCII(const std::string letter, const int single_or_combination);

	std::string convertASCIIToLetters(int number,int single_or_combination);

	void get_folder_list(std::string path, std::vector<std::string>& folder_list);

	void get_image_file_list(std::string path, std::vector<std::string>& image_file_list);

	void get_feature_descriptor_from_training_data(const std::string path_training_data_files, int number_or_letter, int feature_type,
			int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels);

	void compute_descriptor_pyramid(const cv::Mat& image, const int label, int feature_type, cv::Mat& train_data, cv::Mat& train_labels);

	void load_symbol_templates_and_generate_training_data(std::string template_path, int letter_or_number, int feature_type,
			cv::Mat& train_data, cv::Mat& train_labels);

	void load_all_training_data_with_feature_descriptors(std::string training_path, int letter_or_number, int feature_type,
			int training_data_source, int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels);

	void load_training_data(std::string path_data, int feature_type, int training_data_source, int single_or_combination,
			cv::Mat& numbers_train_data, cv::Mat& numbers_train_labels, cv::Mat& letters_train_data, cv::Mat& letters_train_labels);

	cv::Mat get_descriptor_from_text_tag(const cv::Mat& tag_image, const int feature_number, const int single_or_combination);

	std::string convert_classification_labels_to_string(const std::vector<int>& classification_labels, const int single_or_combination);

	std::string read_text_tag_SVM(const cv::SVM& numbers_svm, const cv::SVM& letters_svm, const cv::Mat& tag_image, const int feature_type, const int single_or_combination);
	std::string read_text_tag_KNN(const cv::KNearest& numbers_knn, const cv::KNearest& letters_knn, const cv::Mat& tag_image, const int feature_type, const int single_or_combination);

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
