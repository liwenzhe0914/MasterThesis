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


cv::Mat FeatureReprenstation::get_feature_descriptor(cv::Mat img,int feature_number)
{
	//read image file
	cv::Mat img_gray,descriptorsValues;
	if(feature_number==1)
	{
		//resizing
		cv::resize(img, img, cv::Size(64,64) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		//gray
		cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
		cv::HOGDescriptor d( cv::Size(64,64), cv::Size(16,16),cv:: Size(8,8), cv::Size(8,8), 9);
		cv::vector< float> descriptorsValues_vector;
		cv::vector< cv::Point> locations;
		d.compute( img_gray, descriptorsValues_vector, cv::Size(0,0), cv::Size(0,0), locations);

		cv::Mat descriptorsValues(1, descriptorsValues_vector.size(),CV_32FC1);
		//convert descriptorsValues(type vector) to row matrix
		memcpy(descriptorsValues.data,descriptorsValues_vector.data(),descriptorsValues_vector.size()*sizeof(float));
	}
	else if(feature_number==2)
	{
		cv::Mat lbp_image;
		//resizing
		cv::resize(img, img, cv::Size(64,64) ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		//gray
		cv::cvtColor(img, img_gray, CV_RGB2GRAY);
		lbp::OLBP(img_gray,lbp_image);
		cv::normalize(lbp_image, lbp_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

		descriptorsValues=lbp::spatial_histogram(lbp_image, 59 , 8,8, true);
	}
	else if (feature_number==3)
	{
		cv::Mat descriptorsValues_Mat;
		cv::resize(img, img, cv::Size(57,57)); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		cv::DenseFeatureDetector detector(12.0f, 1, 0.1f, 10);
		cv::BriefDescriptorExtractor extractor(32);
		std::vector<cv::KeyPoint> keyPoints;

		//gray
		cvtColor(img, img_gray, CV_RGB2GRAY,CV_32FC1);
		cv::KeyPoint pt(float(img_gray.cols/2),float(img_gray.rows/2),1,-1, 0, 0, -1);
		keyPoints.push_back(pt);

	//	detector.detect(img_gray,keyPoints);
		extractor.compute(img_gray,keyPoints,descriptorsValues_Mat);
		std::vector<float> descriptorsValues_vector;
		cv::Mat descriptorsValues;

		descriptorsValues_vector.assign(descriptorsValues_Mat.datastart, descriptorsValues_Mat.dataend);

		cv::Mat descriptorsValues(1, descriptorsValues_vector.size(),CV_32FC1);
		//convert descriptorsValues(type vector) to row matrix
		memcpy(descriptorsValues.data,descriptorsValues_vector.data(),descriptorsValues_vector.size()*sizeof(float));
	}

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
	std::vector< cv::Point> locations;
	cv::Mat descriptorsValues;
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

			descriptorsValues=get_feature_descriptor(img,feature_number);
			trainData.push_back(descriptorsValues);
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

int FeatureReprenstation::convertLettersToASCII(std::string letter)
{
	int class_number;
	char label1 = letter.at(0);
	char label2 = letter.at(1);
	int class_number_temp = (int(label1)-64)*10 + (int(label2)-64);
	class_number=class_number_temp-10+(int(label1)-65)*16;
    return class_number;
}

std::string FeatureReprenstation::convertASCIIToLetters(int number)
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

cv::Mat FeatureReprenstation::load_all_training_data_with_feature_descriptors(std::string training_path,int number_or_letter,int feature_number, int load)
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

cv::Mat FeatureReprenstation::preprocess_test_text_tag(cv::Mat testImg,int feature_number)
{
	std::vector<cv::Mat> image_portions;
	cv:: Mat test_descriptorsValues;

	cv::Mat letter_portion = testImg(cv::Rect(0,0, testImg.cols*0.25, testImg.rows));
	image_portions.push_back(letter_portion);
	for (double x_min_ratio = 0.25; x_min_ratio < 1.0; x_min_ratio += 0.25)
	{
		cv::Mat number_portion = testImg(cv::Rect(testImg.cols*x_min_ratio,0, testImg.cols*0.25, testImg.rows));
		image_portions.push_back(number_portion);
	}

	for (unsigned int i = 0; i<4;i++)
	{
		test_descriptorsValues.push_back(get_feature_descriptor(image_portions[i],feature_number));
	}

return test_descriptorsValues;
}

std::string FeatureReprenstation::read_text_tag(cv::Mat testImg,int load,int classifier,int feature_number)
{
	//classifier: 1. KNN 2. train SVM 3. load SVM
	std::string suffix,prefix;
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
	cv::Mat test_descriptorsValues=preprocess_test_text_tag(testImg,feature_number);

	double start_time;
	double time_in_seconds;

	std::string training_path = "/home/damon/training_samples_for_SVM/";

	cv::Mat numbers_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors(training_path,1,feature_number,load);
	cv::Mat letters_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors(training_path,0,feature_number,load);

	cv::Mat numbers_trainData (numbers_trainData_and_trainClasses,cv::Rect(0,0,numbers_trainData_and_trainClasses.cols-1,numbers_trainData_and_trainClasses.rows));
	cv::Mat letters_trainData (letters_trainData_and_trainClasses,cv::Rect(0,0,letters_trainData_and_trainClasses.cols-1,letters_trainData_and_trainClasses.rows));
	cv::Mat numbers_trainClasses = numbers_trainData_and_trainClasses.col(numbers_trainData_and_trainClasses.cols-1);
	cv::Mat letters_trainClasses = letters_trainData_and_trainClasses.col(letters_trainData_and_trainClasses.cols-1);

	std::cout<<"finish processing training data..."<<std::endl;

	// the result of letter combinations.(at first they were ASCII numbers)
	std::vector<int> text_label_result_int;


	if (classifier==3 || classifier==2)
	{
		cv::SVM numbers_svm,letters_svm;
		std::stringstream ss;
		std::string number_svm_model,letter_svm_model;
		prefix="SVM";
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

		char *cstr_number,*cstr_letter;
		cstr_number = new char[number_svm_model.length() + 1];
		cstr_letter = new char[letter_svm_model.length() + 1];
		strcpy(cstr_number, number_svm_model.c_str());
		strcpy(cstr_letter, letter_svm_model.c_str());

		ss<<"common/files/number_svm_model_"<<suffix<<".xml";
		std::string number_svm_model = ss.str();
		ss.str("");
		ss<<"common/files/letters_svm_model_"<<suffix<<".xml";
		std::string letter_svm_model = ss.str();
		ss.str("");

		if(classifier==3)
		{
			std::cout<<"loading SVM classifiers"<<std::endl;
			start_time=clock();
			numbers_svm.load(cstr_number);
			letters_svm.load(cstr_letter);
			time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
			std::cout << "[" << time_in_seconds << " s] processing time for loading 2 SVMs." << std::endl;
		}
		else if (classifier==2)
		{
			std::cout<<"training SVM classifiers"<<std::endl;

			///train SVM with trainData and trainClasses
			cv::SVMParams params;
			params.svm_type=cv::SVM::C_SVC;
			params.C=1;
			params.kernel_type=cv::SVM::LINEAR;
			params.term_crit=cv::TermCriteria(CV_TERMCRIT_ITER, (int)1e7, 1e-6);
			start_time = clock();

			std::cout<<"training SVM... "<<std::endl;
			numbers_svm.train(numbers_trainData,numbers_trainClasses, cv::Mat(), cv::Mat(),params);
			letters_svm.train(letters_trainData,letters_trainClasses, cv::Mat(), cv::Mat(),params);

			time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
			std::cout << "[" << time_in_seconds << " s] processing time for training SVM" << std::endl;

			numbers_svm.save(cstr_number);
			letters_svm.save(cstr_letter);

			std::cout << "SVM training model Saved" << std::endl;
		}
		delete [] cstr_number,cstr_letter;

		float response = letters_svm.predict(test_descriptorsValues.row(0),test_descriptorsValues.row(0).cols);
		text_label_result_int.push_back(response);

		for (unsigned int j =0;j<4;j++)
		{
			float response = numbers_svm.predict(test_descriptorsValues.row(j),test_descriptorsValues.row(j).cols);
			text_label_result_int.push_back(response);
		}
	}
	else if (classifier==1)
	{
		prefix="KNN";

		//Build KNNs

		cv::KNearest numbers_knn(numbers_trainData,numbers_trainClasses);
		cv::KNearest letters_knn(letters_trainData,letters_trainClasses);

		cv::Mat results(1,1,CV_32FC1);
		cv::Mat neighbourResponses = cv::Mat::ones(1,10,CV_32FC1);
		cv::Mat dist = cv::Mat::ones(1, 10, CV_32FC1);

		//strat training

		std::cout<<"training KNN classifiers wit HOG"<<std::endl;
		start_time = clock();
		numbers_knn.train(numbers_trainData, numbers_trainClasses);
		letters_knn.train(letters_trainData, letters_trainClasses);
		time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
		std::cout << "[" << time_in_seconds << " s] processing time for training KNN" << std::endl;


		letters_knn.find_nearest(test_descriptorsValues.row(0),1,results, neighbourResponses, dist);
		text_label_result_int.push_back(results.at<float>(0,0));

		for (unsigned int j =1;j<4;j++)
		{
			numbers_knn.find_nearest(test_descriptorsValues.row(j),1,results, neighbourResponses, dist);
			text_label_result_int.push_back(results.at<float>(0,0));
		}
	}

	std::stringstream ss;
	ss<<convertASCIIToLetters(text_label_result_int[0])<<"-"<<text_label_result_int[1]
	                    <<"-"<<text_label_result_int[2]<<"-"<<text_label_result_int[3];
	std::string text_label = ss.str();
	ss.str("");
	std::cout << "From classifier $"<<prefix<<"$ with the feature $"<<suffix <<"$ got label = "<<text_label<< std::endl;

	return text_label;
}



