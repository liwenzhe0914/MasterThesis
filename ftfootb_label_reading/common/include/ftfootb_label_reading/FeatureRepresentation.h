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

		cv::Mat get_feature_descriptor(cv::Mat& img,int feature_number,int single_or_combination);

		int convertLettersToASCII(std::string letter,int single_or_combination);

		std::string convertASCIIToLetters(int number,int single_or_combination);

		std::vector<std::string> folder_list(std::string path);

		std::vector<std::string> load_folder_of_image(std::string path);

		cv::Mat get_feature_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,
																int number_or_letter,int feature_number,int single_or_combination);

		cv::Mat load_all_training_data_with_feature_descriptors(std::string training_path,int number_or_letter,
																int feature_number,int load,int single_or_combination);

		cv::Mat preprocess_test_text_tag(cv::Mat& testImg,int feature_number, int single_or_combination);

		std::string read_text_tag_SVM(cv::SVM& numbers_svm,cv::SVM& letters_svm,cv::Mat& image,int feature_number,int single_or_combination);
		std::string read_text_tag_KNN(cv::KNearest& numbers_knn,cv::KNearest& letters_knn,cv::Mat& testImg,int feature_number,int single_or_combination);
		cv::Mat wolf_thresholding(cv::Mat& img_gray);

		void help();

		void load_or_train_SVM_classifiers(cv::SVM& numbers_svm,cv::SVM& letters_svm,
											int load,int classifier,int feature_number,int single_or_combination,
											std::string path_data);

		void load_or_train_KNN_classifiers(cv::KNearest& numbers_knn,cv::KNearest& letters_knn,
											int load,int classifier,int feature_number,int single_or_combination,
											std::string path_data);

		cv::SVM numbers_svm;
		cv::SVM letters_svm;
		cv::KNearest numbers_knn,letters_knn;
};
