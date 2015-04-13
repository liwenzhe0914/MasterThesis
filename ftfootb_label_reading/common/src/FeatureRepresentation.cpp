#include "ftfootb_label_reading/FeatureRepresentation.h"


std::vector<std::string> FeatureReprenstation::folder_list(std::string path)
{
	std::vector<std::string> FolderNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
	{
	  /* print all the files and directories within directory */
		while ((entry = readdir(pDIR)) != NULL)
		{
			std::string completeName = entry->d_name;
			std::string name_end = completeName.substr(completeName.find_last_of("/") + 1, completeName.length());
			std::string s = path;
			if (name_end !="." and name_end !="..")
				{
				s.append(completeName);
				FolderNames.push_back(s);
				}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
return FolderNames;
}


std::vector<std::string> FeatureReprenstation::load_folder_of_image(std::string path)
{
	std::vector<std::string> ImageNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
	{
		while ((entry = readdir(pDIR)) != NULL)
		{
			if (std::strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				std::string completeName = entry->d_name;
				std::string imgend = completeName.substr(completeName.find_last_of(".") + 1, completeName.length() - completeName.find_last_of("."));
				if (std::strcmp(imgend.c_str(), "png") == 0 || std::strcmp(imgend.c_str(), "PNG") == 0
						|| std::strcmp(imgend.c_str(), "JPG") == 0 || std::strcmp(imgend.c_str(), "jpg") == 0
						|| std::strcmp(imgend.c_str(), "jpeg") == 0 || std::strcmp(imgend.c_str(), "Jpeg") == 0
						|| std::strcmp(imgend.c_str(), "bmp") == 0 || std::strcmp(imgend.c_str(), "BMP") == 0
						|| std::strcmp(imgend.c_str(), "TIFF") == 0 || std::strcmp(imgend.c_str(), "tiff") == 0
						|| std::strcmp(imgend.c_str(), "tif") == 0 || std::strcmp(imgend.c_str(), "TIF") == 0)
				{
					std::string s = path;
					if (s.at(s.length() - 1) != '/')
						s.append("/");
					s.append(entry->d_name);
					ImageNames.push_back(s);
				}
			}
		}
	}
	else
	{
		std::cout << "Error: Could not open path." << std::endl;
	}
return ImageNames;
}


std::vector<float> FeatureReprenstation::get_HOG_descriptor(cv::Mat img)
{
	//read image file
	cv::Mat img_gray;

	//resizing
	cv::resize(img, img, cv::Size(64,64) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
	//gray
	cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
	cv::HOGDescriptor d( cv::Size(64,64), cv::Size(16,16),cv:: Size(8,8), cv::Size(8,8), 9);
	cv::vector< float> descriptorsValues;
	cv::vector< cv::Point> locations;
	d.compute( img_gray, descriptorsValues, cv::Size(0,0), cv::Size(0,0), locations);

	return descriptorsValues;
}

cv::Mat FeatureReprenstation::get_LBP_descriptor(cv::Mat img)
{
	//read image file
	cv::Mat img_gray,lbp_image,hist;

	//resizing
	cv::resize(img, img, cv::Size(64,64) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
	//gray
	cv::cvtColor(img, img_gray, CV_RGB2GRAY);
	lbp::OLBP(img_gray,lbp_image);
	cv::normalize(lbp_image, lbp_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

	hist=lbp::spatial_histogram(lbp_image, 59 , 8,8, true);

	return hist;
}

std::vector<float> FeatureReprenstation::get_BRIEF_descriptor(cv::Mat img)
{
	cv::DenseFeatureDetector detector(12.0f, 1, 0.1f, 10);
	cv::BriefDescriptorExtractor extractor(32);
	std::vector<cv::KeyPoint> keyPoints;

	cv::Mat descriptorsValues_Mat;
	//read image file
	cv::Mat img_gray;

	//resizing
	cv::resize(img, img, cv::Size(57,57)); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );

	//gray
	cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
	cv::KeyPoint pt(float(img_gray.cols/2),float(img_gray.rows/2),1,-1, 0, 0, -1);
	keyPoints.push_back(pt);

//	detector.detect(img_gray,keyPoints);
	extractor.compute(img_gray,keyPoints,descriptorsValues_Mat);
	std::vector<float> descriptorsValues;

	descriptorsValues.assign(descriptorsValues_Mat.datastart, descriptorsValues_Mat.dataend);

	return descriptorsValues;
}

cv::Mat FeatureReprenstation::get_feature_descriptor_from_training_data(std::vector<std::string> FoldersFullNames,int number_or_letter,int feature_number)
{
	//feature_number: 1.HOG 2.LBP 3.BRIEF
	//number_or_letter: 1. number 2. letter
	char FullFileName[100];
	std::string foldersNames;
	cv::Mat trainData,trainClasses;
	char *cstr;
	std::vector<std::string> ImageNames;
	std::vector< float> descriptorsValues;
	std::vector< cv::Point> locations;
	cv::Mat LBP_hist;
	int Classes;
	std::ofstream out_file;

	for(unsigned int i=0; i< FoldersFullNames.size(); ++i)
	{
		std::string foldername_temp = FoldersFullNames.at(i);
		foldersNames = foldername_temp.substr(foldername_temp.find_last_of("/") + 1, foldername_temp.length());
		std::stringstream ss1;
		ss1<<foldername_temp<<"/"<<foldersNames<<"_";
		std::string FirstFileName = ss1.str();
		ss1.str("");
		cstr = new char[FirstFileName.length() + 1];
		strcpy(cstr, FirstFileName.c_str());
		ImageNames = load_folder_of_image(foldername_temp);
		int FileNum = ImageNames.size();
		for(int k=0; k< FileNum; ++k)
		{
			sprintf(FullFileName, "%s%d.png", cstr, k+1);

			cv::Mat img = cv::imread(FullFileName);
			if (feature_number==1)
			{
				descriptorsValues=get_HOG_descriptor(img);

				// convert descriptorsValues vector to a row
				cv::Mat row(1, descriptorsValues.size(),CV_32FC1);
				memcpy(row.data,descriptorsValues.data(),descriptorsValues.size()*sizeof(float));
				trainData.push_back(row);
				if (number_or_letter == 1)
				{
					istringstream ( foldersNames ) >> Classes;
				}
				else
				{
					Classes = convertLettersToASCII(foldersNames);
				}
			trainClasses.push_back( Classes );
			}
			else if (feature_number==2)
				{
					LBP_hist=get_LBP_descriptor(img);
					trainData.push_back(LBP_hist);
					if (number_or_letter == 1)
					{
						istringstream ( foldersNames ) >> Classes;
					}
					else
					{
						Classes = convertLettersToASCII(foldersNames);
					}
					trainClasses.push_back( Classes );
				}
			else if (feature_number==3)
			{
				descriptorsValues=get_BRIEF_descriptor(img);

				// convert descriptorsValues vector to a row
				cv::Mat row(1, descriptorsValues.size(),CV_32FC1);
				memcpy(row.data,descriptorsValues.data(),descriptorsValues.size()*sizeof(float));
				trainData.push_back(row);
				if (number_or_letter == 1)
				{
					istringstream ( foldersNames ) >> Classes;
				}
				else
				{
					Classes = convertLettersToASCII(foldersNames);
				}
			}
		}
	}
	delete [] cstr;

	cv::Mat trainData_trainClasses(trainData.rows,trainData.cols+trainClasses.cols,CV_32FC1,cv::Scalar::all(0));

///combine two Matrix -- trainData and trainClasses into one Matrix
	for (int i = 1;i<trainData.cols;i++)
	{
		trainData.col(i).copyTo(trainData_trainClasses.col(i));
	}
//	cout<<trainClasses.col(0)<<endl;
	trainClasses.col(0).copyTo(trainData_trainClasses.col(trainData.cols+trainClasses.cols-1));

	return trainData_trainClasses;//trainData_trainClasses contains trainData and trainClasses
}

int convertLettersToASCII(std::string letter)
{
	int class_number;
	char label1 = letter.at(0);
	char label2 = letter.at(1);
	int class_number_temp = (int(label1)-64)*10 + (int(label2)-64);
	class_number=class_number_temp-10+(int(label1)-65)*16;
    return class_number;
}

std::string convertASCIIToLetters(int number)
{
	int ASCII_1st_letter;
	int remainder = number%26;
	std::string letter_class;

	if (remainder !=0)
	{
		ASCII_1st_letter = floor(number/26)+65;
		std::stringstream ss;
		ss<<char(ASCII_1st_letter)<<char(remainder+64);
		letter_class = ss.str();
		ss.str("");
	}
	else
	{
		ASCII_1st_letter = floor(number/26)+64;
		std::stringstream ss;
		ss<<char(ASCII_1st_letter)<<char(26+64);
		letter_class = ss.str();
		ss.str("");
	}
    return letter_class;
}

cv::Mat load_all_training_data_with_feature_descriptors(std::string training_path,int number_or_letter,int feature_number, int load)
{
	//load: 1. load data from yml. 0. load from raw data


	cv::Mat trainData_and_trainClasses;
	std::string prefix,suffix;
	std::stringstream ss_numbers;
	ss_numbers<<training_path<<"numbers"<<"/";
	std::string path_numbers = ss_numbers.str();
	ss_numbers.str("");
	std::stringstream ss_letters;
	ss_letters<<training_path<<"letters"<<"/";
	std::string path_letters = ss_letters.str();
	ss_letters.str("");

	std::stringstream ss;
	std::vector<std::string> TrainingFoldersFullNames;
	if (number_or_letter==1)
	{
		TrainingFoldersFullNames = folder_list(path_numbers);
		std::string prefix = "numbers";
	}
	else if (number_or_letter==0)
	{
		TrainingFoldersFullNames = folder_list(path_letters);
		std::string prefix = "letters";
	}
	if (feature_number==1)
	{
		suffix = "HOG";
	}
	else if (feature_number==2)
	{
		suffix = "LBP";
	}
	else if (feature_number==3)
	{
		suffix = "BRIEF";
	}

if (load==0)
{

	std::cout<<"computing trainData_and_trainClasses from training dataset."<<std::endl;


	ss<<"common/files/"<<prefix<<"_numbers_trainData_and_trainClasses_"<<suffix<<".yml";
	trainData_and_trainClasses=get_HOG_descriptor_from_training_data(TrainingFoldersFullNames,number_or_letter);

	cv::FileStorage fs(ss, cv::FileStorage::WRITE);
	fs << prefix<<"_numbers_trainData_and_trainClasses_"<<suffix << trainData_and_trainClasses;
	ss.str("");
	fs.release();
}
else if (load==1)
{
	std::cout<<"Reading trainData_and_trainClasses from feature description files."<<std::endl;
	ss<<"common/files/"<<prefix<<"_numbers_trainData_and_trainClasses_"<<suffix<<".yml";
	cv::FileStorage fs(ss, cv::FileStorage::READ);
	fs[prefix<<"_numbers_trainData_and_trainClasses_"<<suffix] >> trainData_and_trainClasses;
	fs.release();
}

return trainData_and_trainClasses;
}

std::string read_text_tag(cv::Mat testImg,int load)
{
	std::string training_path = "/home/damon/training_samples_for_SVM/";

	cv::Mat numbers_trainData_and_trainClasses_HOG = load_all_training_data_with_feature_descriptors(training_path,1,1,load);
	cv::Mat letters_trainData_and_trainClasses_HOG = load_all_training_data_with_feature_descriptors(training_path,0,1,load);
	cv::Mat numbers_trainData_and_trainClasses_BRIEF = load_all_training_data_with_feature_descriptors(training_path,1,1,load);
	cv::Mat letters_trainData_and_trainClasses_BRIEF = load_all_training_data_with_feature_descriptors(training_path,0,1,load);
	cv::Mat numbers_trainData_and_trainClasses_LBP = load_all_training_data_with_feature_descriptors(training_path,1,1,load);
	cv::Mat letters_trainData_and_trainClasses_LBP = load_all_training_data_with_feature_descriptors(training_path,0,1,load);


	cv::Mat numbers_trainData_HOG (numbers_trainData_and_trainClasses_HOG,cv::Rect(0,0,numbers_trainData_and_trainClasses_HOG.cols-1,numbers_trainData_and_trainClasses_HOG.rows));
	cv::Mat letters_trainData_HOG (letters_trainData_and_trainClasses_HOG,cv::Rect(0,0,letters_trainData_and_trainClasses_HOG.cols-1,letters_trainData_and_trainClasses_HOG.rows));
	cv::Mat numbers_trainClasses_HOG = numbers_trainData_and_trainClasses_HOG.col(numbers_trainData_and_trainClasses_HOG.cols-1);
	cv::Mat letters_trainClasses_HOG = letters_trainData_and_trainClasses_HOG.col(letters_trainData_and_trainClasses_HOG.cols-1);

	cv::Mat numbers_trainData_BRIEF (numbers_trainData_and_trainClasses_BRIEF,cv::Rect(0,0,numbers_trainData_and_trainClasses_BRIEF.cols-1,numbers_trainData_and_trainClasses_BRIEF.rows));
	cv::Mat letters_trainData_BRIEF (letters_trainData_and_trainClasses_BRIEF,cv::Rect(0,0,letters_trainData_and_trainClasses_BRIEF.cols-1,letters_trainData_and_trainClasses_BRIEF.rows));
	cv::Mat numbers_trainClasses_BRIEF=numbers_trainData_and_trainClasses_BRIEF.col(numbers_trainData_and_trainClasses_BRIEF.cols-1);
	cv::Mat letters_trainClasses_BRIEF=letters_trainData_and_trainClasses_BRIEF.col(letters_trainData_and_trainClasses_BRIEF.cols-1);

	cv::Mat numbers_trainData_LBP (numbers_trainData_and_trainClasses_LBP,cv::Rect(0,0,numbers_trainData_and_trainClasses_LBP.cols-1,numbers_trainData_and_trainClasses_LBP.rows));
	cv::Mat letters_trainData_LBP (letters_trainData_and_trainClasses_LBP,cv::Rect(0,0,letters_trainData_and_trainClasses_LBP.cols-1,letters_trainData_and_trainClasses_LBP.rows));
	cv::Mat numbers_trainClasses_LBP=numbers_trainData_and_trainClasses_LBP.col(numbers_trainData_and_trainClasses_LBP.cols-1);
	cv::Mat letters_trainClasses_LBP=letters_trainData_and_trainClasses_LBP.col(letters_trainData_and_trainClasses_LBP.cols-1);

	std::cout<<"finish processing training data..."<<std::endl;


















	std::vector< float> descriptorsValues_HOG,descriptorsValues_BRIEF;
	cv:: Mat descriptorsValues_LBP;

	std::vector<cv::Mat> image_portions;
	cv::Mat letter_portion = testImg(cv::Rect(0,0, testImg.cols*0.25, testImg.rows));
	image_portions.push_back(letter_portion);
	for (double x_min_ratio = 0.25; x_min_ratio < 1.0; x_min_ratio += 0.25)
	{
		cv::Mat number_portion = testImg(cv::Rect(testImg.cols*x_min_ratio,0, testImg.cols*0.25, testImg.rows));
		image_portions.push_back(number_portion);
	}

//	descriptorsValues_HOG=get_HOG_descriptor(image_portions[0]);
//	descriptorsValues_LBP=get_LBP_descriptor(image_portions[0]);
//	descriptorsValues_BRIEF=get_BRIEF_descriptor(image_portions[0]);
//
//	for(unsigned int i=1; i<= 3; ++i)
//	{
//		descriptorsValues_HOG=get_HOG_descriptor(image_portions[i]);
//		descriptorsValues_BRIEF=get_BRIEF_descriptor(image_portions[i]);
//		descriptorsValues_LBP=get_LBP_descriptor(image_portions[i]);
//	}

	//write descritorValues of 4 portions(4 row matrix)
	cv::Mat test_descriptorsValues_HOG;
	cv::Mat row_temp_HOG(1, descriptorsValues_HOG.size(),CV_32FC1);
	cv::Mat test_descriptorsValues_BRIEF;
	cv::Mat row_temp_BRIEF(1, descriptorsValues_BRIEF.size(),CV_32FC1);
	cv::Mat test_descriptorsValues_LBP;
	cv::Mat row_temp_LBP(descriptorsValues_LBP.size(),CV_32FC1);

	// write text portions to test_descriptorsValues Matrix, respectively
	std::vector< float> test_descriptorsValues_temp_HOG;
	test_descriptorsValues_temp_HOG = get_HOG_descriptor(image_portions[0]);
	memcpy(row_temp_HOG.data,test_descriptorsValues_temp_HOG.data(),test_descriptorsValues_temp_HOG.size()*sizeof(float));//convert descriptorsValues(type vector) to row matrix
	test_descriptorsValues_HOG.push_back(row_temp_HOG);

	std::vector< float> test_descriptorsValues_temp_BRIEF;
	test_descriptorsValues_temp_BRIEF = get_BRIEF_descriptor(image_portions[0]);
	memcpy(row_temp_BRIEF.data,test_descriptorsValues_temp_BRIEF.data(),test_descriptorsValues_temp_BRIEF.size()*sizeof(float));//convert descriptorsValues(type vector) to row matrix
	test_descriptorsValues_BRIEF.push_back(row_temp_BRIEF);

	row_temp_LBP=get_LBP_descriptor(image_portions[0]);
	test_descriptorsValues_LBP.push_back(row_temp_LBP);

	for (unsigned int j = 1;j<4;j++)
	{
		test_descriptorsValues_temp_HOG = get_HOG_descriptor(image_portions[j]);
		memcpy(row_temp_HOG.data,test_descriptorsValues_temp_HOG.data(),test_descriptorsValues_temp_HOG.size()*sizeof(float));
		test_descriptorsValues_HOG.push_back(row_temp_HOG);

		test_descriptorsValues_temp_BRIEF = get_BRIEF_descriptor(image_portions[j]);
		memcpy(row_temp_BRIEF.data,test_descriptorsValues_temp_BRIEF.data(),test_descriptorsValues_temp_BRIEF.size()*sizeof(float));
		test_descriptorsValues_BRIEF.push_back(row_temp_BRIEF);

		row_temp_LBP=get_LBP_descriptor(image_portions[j]);
		test_descriptorsValues_LBP.push_back(row_temp_LBP);
	}

	for (unsigned int j = 1;j<4;j++)
	{
		test_descriptorsValues_temp_HOG = get_HOG_descriptor(image_portions[j]);
		//test_descriptorsValues_temp_BRIEF = get_BRIEF_descriptor(image_portions[j]);
		memcpy(row_temp_HOG.data,test_descriptorsValues_temp_HOG.data(),test_descriptorsValues_temp_HOG.size()*sizeof(float));
		//memcpy(row_temp_BRIEF.data,test_descriptorsValues_temp_BRIEF.data(),test_descriptorsValues_temp_BRIEF.size()*sizeof(float));
		test_descriptorsValues_HOG.push_back(row_temp_HOG);
		//test_descriptorsValues_BRIEF.push_back(row_temp_BRIEF);
		row_temp_LBP=get_LBP_descriptor(image_portions[j]);
		test_descriptorsValues_LBP.push_back(row_temp_LBP);
	}

	//Build result function -- knn.find_nearest
	cv::Mat results_HOG(1,1,CV_32FC1);
	cv::Mat neighbourResponses_HOG = cv::Mat::ones(1,10,CV_32FC1);
	cv::Mat dist_HOG = cv::Mat::ones(1, 10, CV_32FC1);

	cv::Mat results_BRIEF(1,1,CV_32FC1);
	cv::Mat neighbourResponses_BRIEF = cv::Mat::ones(1,10,CV_32FC1);
	cv::Mat dist_BRIEF = cv::Mat::ones(1, 10, CV_32FC1);

	cv::Mat results_LBP(1,1,CV_32FC1);
	cv::Mat neighbourResponses_LBP = cv::Mat::ones(1,10,CV_32FC1);
	cv::Mat dist_LBP = cv::Mat::ones(1, 10, CV_32FC1);

	///////////////  KNN ///////////
	letters_knn_HOG.find_nearest(test_descriptorsValues_HOG.row(0),1,results_HOG, neighbourResponses_HOG, dist_HOG);
	text_label_result_KNN_int_HOG.push_back(results_HOG.at<float>(0,0));

	//letters_knn_BRIEF.find_nearest(test_descriptorsValues_BRIEF.row(0),5,results_BRIEF, neighbourResponses_BRIEF, dist_BRIEF);
	//text_label_result_KNN_int_BRIEF.push_back(results_BRIEF.at<float>(0,0));

	letters_knn_LBP.find_nearest(test_descriptorsValues_LBP.row(0),1,results_LBP, neighbourResponses_LBP, dist_LBP);
	text_label_result_KNN_int_LBP.push_back(results_LBP.at<float>(0,0));

	float response_HOG = letters_svm_HOG.predict(test_descriptorsValues_HOG.row(0),test_descriptorsValues_HOG.row(0).cols);
	text_label_result_SVM_int_HOG.push_back(response_HOG);

	//float response_BRIEF = letters_svm_BRIEF.predict(test_descriptorsValues_BRIEF.row(0),test_descriptorsValues_BRIEF.row(0).cols);
	//text_label_result_SVM_int_BRIEF.push_back(response_BRIEF);

	float response_LBP = letters_svm_LBP.predict(test_descriptorsValues_LBP.row(0),test_descriptorsValues_LBP.row(0).cols);
	text_label_result_SVM_int_LBP.push_back(response_LBP);

	for (unsigned int j =1;j<4;j++)
	{
		numbers_knn_HOG.find_nearest(test_descriptorsValues_HOG.row(j),1,results_HOG, neighbourResponses_HOG, dist_HOG);
		text_label_result_KNN_int_HOG.push_back(results_HOG.at<float>(0,0));

		numbers_knn_LBP.find_nearest(test_descriptorsValues_LBP.row(j),1,results_LBP, neighbourResponses_LBP, dist_LBP);
		text_label_result_KNN_int_LBP.push_back(results_LBP.at<float>(0,0));

		/*numbers_knn_BRIEF.find_nearest(test_descriptorsValues_BRIEF.row(j),5,results_BRIEF, neighbourResponses_BRIEF, dist_BRIEF);
		text_label_result_KNN_int_BRIEF.push_back(results_BRIEF.at<float>(0,0));*/

		float response_HOG = numbers_svm_HOG.predict(test_descriptorsValues_HOG.row(j),test_descriptorsValues_HOG.row(j).cols);
		text_label_result_SVM_int_HOG.push_back(response_HOG);

		float response_LBP = numbers_svm_LBP.predict(test_descriptorsValues_LBP.row(j),test_descriptorsValues_LBP.row(j).cols);
		text_label_result_SVM_int_LBP.push_back(response_LBP);

		/*float response_BRIEF = numbers_svm_BRIEF.predict(test_descriptorsValues_BRIEF.row(j),test_descriptorsValues_BRIEF.row(j).cols);
		text_label_result_SVM_int_BRIEF.push_back(response_BRIEF);*/
	}

	std::string text_label;





























	return text_label;
}



