#include "ftfootb_label_reading/FeatureRepresentation.h"






//-------------------------- Wolf Thresholding ---------------------------------------------------
/**************************************************************
 * Binarization with several methods
 * (0) Niblacks method
 * (1) Sauvola & Co.
 *     ICDAR 1997, pp 147-152
 * (2) by myself - Christian Wolf
 *     Research notebook 19.4.2001, page 129
 * (3) by myself - Christian Wolf
 *     20.4.2007
 *
 * See also:
 * Research notebook 24.4.2001, page 132 (Calculation of s)
 **************************************************************/

std::string getImageType(int number)
{
    // find type
    int imgTypeInt = number%8;
    std::string imgTypeString;

    switch (imgTypeInt)
    {
        case 0:
            imgTypeString = "8U";
            break;
        case 1:
            imgTypeString = "8S";
            break;
        case 2:
            imgTypeString = "16U";
            break;
        case 3:
            imgTypeString = "16S";
            break;
        case 4:
            imgTypeString = "32S";
            break;
        case 5:
            imgTypeString = "32F";
            break;
        case 6:
            imgTypeString = "64F";
            break;
        default:
            break;

    }
    // find channel
    int channel = (number/8) + 1;

    std::stringstream type;
    type<<"CV_"<<imgTypeString<<"C"<<channel;

    return type.str();
}

enum NiblackVersion
{
	NIBLACK=0,
    SAUVOLA,
    WOLFJOLION,
};

#define BINARIZEWOLF_VERSION	"2.4 (August 1st, 2014)"

#define uget(x,y)    at<unsigned char>(y,x)
#define uset(x,y,v)  at<unsigned char>(y,x)=v;
#define fget(x,y)    at<float>(y,x)
#define fset(x,y,v)  at<float>(y,x)=v;

/**********************************************************
 * Usage
 **********************************************************/


// *************************************************************
// glide a window across the image and
// create two maps: mean and standard deviation.
//
// Version patched by Thibault Yohan (using opencv integral images)
// *************************************************************


double calcLocalStats (cv::Mat &im, cv::Mat &map_m, cv::Mat &map_s, int winx, int winy) {
    cv::Mat im_sum, im_sum_sq;
    cv::integral(im,im_sum,im_sum_sq,CV_64F);

	double m,s,max_s,sum,sum_sq;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
	double winarea = winx*winy;

	max_s = 0;
	for	(int j = y_firstth ; j<=y_lastth; j++){
		sum = sum_sq = 0;

        sum = im_sum.at<double>(j-wyh+winy,winx) - im_sum.at<double>(j-wyh,winx) - im_sum.at<double>(j-wyh+winy,0) + im_sum.at<double>(j-wyh,0);
        sum_sq = im_sum_sq.at<double>(j-wyh+winy,winx) - im_sum_sq.at<double>(j-wyh,winx) - im_sum_sq.at<double>(j-wyh+winy,0) + im_sum_sq.at<double>(j-wyh,0);

		m  = sum / winarea;
		s  = sqrt ((sum_sq - m*sum)/winarea);
		if (s > max_s) max_s = s;

		map_m.fset(x_firstth, j, m);
		map_s.fset(x_firstth, j, s);

		// Shift the window, add and remove	new/old values to the histogram
		for	(int i=1 ; i <= im.cols-winx; i++) {

			// Remove the left old column and add the right new column
			sum -= im_sum.at<double>(j-wyh+winy,i) - im_sum.at<double>(j-wyh,i) - im_sum.at<double>(j-wyh+winy,i-1) + im_sum.at<double>(j-wyh,i-1);
			sum += im_sum.at<double>(j-wyh+winy,i+winx) - im_sum.at<double>(j-wyh,i+winx) - im_sum.at<double>(j-wyh+winy,i+winx-1) + im_sum.at<double>(j-wyh,i+winx-1);

			sum_sq -= im_sum_sq.at<double>(j-wyh+winy,i) - im_sum_sq.at<double>(j-wyh,i) - im_sum_sq.at<double>(j-wyh+winy,i-1) + im_sum_sq.at<double>(j-wyh,i-1);
			sum_sq += im_sum_sq.at<double>(j-wyh+winy,i+winx) - im_sum_sq.at<double>(j-wyh,i+winx) - im_sum_sq.at<double>(j-wyh+winy,i+winx-1) + im_sum_sq.at<double>(j-wyh,i+winx-1);

			m  = sum / winarea;
			s  = sqrt ((sum_sq - m*sum)/winarea);
			if (s > max_s) max_s = s;

			map_m.fset(i+wxh, j, m);
			map_s.fset(i+wxh, j, s);
		}
	}

	return max_s;
}



	/**********************************************************
	 * The binarization routine
	 **********************************************************/


void NiblackSauvolaWolfJolion (cv::Mat im, cv::Mat output, NiblackVersion version,
	int winx, int winy, double k, double dR) {


	double m, s, max_s;
	double th=0;
	double min_I, max_I;
	int wxh	= winx/2;
	int wyh	= winy/2;
	int x_firstth= wxh;
	int x_lastth = im.cols-wxh-1;
	int y_lastth = im.rows-wyh-1;
	int y_firstth= wyh;
//	int mx, my;

	// Create local statistics and store them in a double matrices
	cv::Mat map_m = cv::Mat::zeros (im.rows, im.cols, CV_32F);
	cv::Mat map_s = cv::Mat::zeros (im.rows, im.cols, CV_32F);
	max_s = calcLocalStats (im, map_m, map_s, winx, winy);

	minMaxLoc(im, &min_I, &max_I);

	cv::Mat thsurf (im.rows, im.cols, CV_32F);

	// Create the threshold surface, including border processing
	// ----------------------------------------------------

	for	(int j = y_firstth ; j<=y_lastth; j++) {

		// NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
		for	(int i=0 ; i <= im.cols-winx; i++) {

			m  = map_m.fget(i+wxh, j);
    		s  = map_s.fget(i+wxh, j);

    		// Calculate the threshold
    		switch (version) {

    			case NIBLACK:
    				th = m + k*s;
    				break;

    			case SAUVOLA:
	    			th = m * (1 + k*(s/dR-1));
	    			break;

    			case WOLFJOLION:
    				th = m + k * (s/max_s-1) * (m-min_I);
    				break;

    			default:
    				std::cerr << "Unknown threshold type in ImageThresholder::surfaceNiblackImproved()\n";
    				exit (1);
    		}

    		thsurf.fset(i+wxh,j,th);

    		if (i==0) {
        		// LEFT BORDER
        		for (int i=0; i<=x_firstth; ++i)
                	thsurf.fset(i,j,th);

        		// LEFT-UPPER CORNER
        		if (j==y_firstth)
        			for (int u=0; u<y_firstth; ++u)
        			for (int i=0; i<=x_firstth; ++i)
        				thsurf.fset(i,u,th);

        		// LEFT-LOWER CORNER
        		if (j==y_lastth)
        			for (int u=y_lastth+1; u<im.rows; ++u)
        			for (int i=0; i<=x_firstth; ++i)
        				thsurf.fset(i,u,th);
    		}

			// UPPER BORDER
			if (j==y_firstth)
				for (int u=0; u<y_firstth; ++u)
					thsurf.fset(i+wxh,u,th);

			// LOWER BORDER
			if (j==y_lastth)
				for (int u=y_lastth+1; u<im.rows; ++u)
					thsurf.fset(i+wxh,u,th);
		}

		// RIGHT BORDER
		for (int i=x_lastth; i<im.cols; ++i)
        	thsurf.fset(i,j,th);

  		// RIGHT-UPPER CORNER
		if (j==y_firstth)
			for (int u=0; u<y_firstth; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);

		// RIGHT-LOWER CORNER
		if (j==y_lastth)
			for (int u=y_lastth+1; u<im.rows; ++u)
			for (int i=x_lastth; i<im.cols; ++i)
				thsurf.fset(i,u,th);
	}
	//cerr << "surface created" << endl;


	for	(int y=0; y<im.rows; ++y)
	for	(int x=0; x<im.cols; ++x)
	{
    	if (im.uget(x,y) >= thsurf.fget(x,y))
    	{
    		output.uset(x,y,255);
    	}
    	else
    	{
    	    output.uset(x,y,0);
    	}
    }
}

cv::Mat FeatureReprenstation::wolf_thresholding(cv::Mat& img_gray)
{

	cv::Size dsize = cv::Size(img_gray.cols,img_gray.rows);
//	char version;
//		int c;
		int winx=0, winy=0;
		float optK=0.5;
		NiblackVersion versionCode;
		versionCode = WOLFJOLION;

	    // Treat the window size
	    if (winx==0||winy==0) {
	        winy = (int) (2.0 * img_gray.rows-1)/3;
	        winx = (int) img_gray.cols-1 < winy ? img_gray.cols-1 : winy;
	        // if the window is too big, than we asume that the image
	        // is not a single text box, but a document page: set
	        // the window size to a fixed constant.
	        if (winx > 100)
	            winx = winy = 40;
	        //cerr << "Setting window size to [" << winx
	        //   << "," << winy << "].\n";
	    }

	    // Threshold
	    cv::Mat image_binarized (img_gray.rows, img_gray.cols, CV_8U);
	    NiblackSauvolaWolfJolion (img_gray, image_binarized, versionCode, winx, winy, optK, 128);

	    resize(image_binarized,image_binarized,dsize);
	return image_binarized;
}
//-------------------------- Above > Wolf binarization  -------------------------------------------------------------------------------------------------------------

//-------------------------- Feature representation (HOG, BRIEF, LBP) ---------------------------------------------------

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
			if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
			{
				std::string completeName = entry->d_name;
				std::string imgend = completeName.substr(completeName.find_last_of(".") + 1, completeName.length() - completeName.find_last_of("."));
				if (strcmp(imgend.c_str(), "png") == 0 || strcmp(imgend.c_str(), "PNG") == 0
						|| strcmp(imgend.c_str(), "JPG") == 0 || strcmp(imgend.c_str(), "jpg") == 0
						|| strcmp(imgend.c_str(), "jpeg") == 0 || strcmp(imgend.c_str(), "Jpeg") == 0
						|| strcmp(imgend.c_str(), "bmp") == 0 || strcmp(imgend.c_str(), "BMP") == 0
						|| strcmp(imgend.c_str(), "TIFF") == 0 || strcmp(imgend.c_str(), "tiff") == 0
						|| strcmp(imgend.c_str(), "tif") == 0 || strcmp(imgend.c_str(), "TIF") == 0)
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
		std::cout << "Error: Could not open path!" << std::endl;
	}
return ImageNames;
}


cv::Rect FeatureReprenstation::remove_white_image_border(const cv::Mat& image, const cv::Rect& roi)
{
	// remove white border
	cv::Mat binary_image;
	cv::threshold(image(roi), binary_image, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Point min_point(binary_image.cols, binary_image.rows);
	cv::Point max_point(0, 0);
	for (int v=0; v<binary_image.rows; ++v)
		for (int u=0; u<binary_image.cols; ++u)
			if (binary_image.at<uchar>(v,u)==0)
			{
				if (u < min_point.x) min_point.x = u;
				if (v < min_point.y) min_point.y = v;
				if (max_point.x < u) max_point.x = u;
				if (max_point.y < v) max_point.y = v;
			}
	cv::Rect bounding_box(roi.x + min_point.x, roi.y + min_point.y, max_point.x-min_point.x+1, max_point.y-min_point.y+1);

//	cv::imshow("padding", image);
//	cv::imshow("binary", binary_image);
//	cv::Mat cut = image(bounding_box);
//	cv::imshow("cut", cut);
//	cv::waitKey();

	return bounding_box;
}


cv::Mat FeatureReprenstation::get_feature_descriptor(const cv::Mat& image, int feature_number, int single_or_combination)
{
	//read image file
	cv::Mat image_grayscale;
	cv::Mat descriptor_values;
	cv::Size dsize_HOG_LBP;
	cv::Size dsize_BRIEF;
	if (single_or_combination==1)
	{
		dsize_HOG_LBP=cv::Size(32,64);
		dsize_BRIEF=cv::Size(57,57);
	}
	else if (single_or_combination==2)
	{
		dsize_HOG_LBP=cv::Size(64,64);
		dsize_BRIEF=cv::Size(57,57);
	}

	if(feature_number==1)
	{

		//resizing
		cv::resize(image, image_grayscale, dsize_HOG_LBP); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );

		cv::HOGDescriptor d( dsize_HOG_LBP, cv::Size(dsize_HOG_LBP.width/4,dsize_HOG_LBP.height/4),
				cv::Size(dsize_HOG_LBP.width/8,dsize_HOG_LBP.height/8), cv::Size(dsize_HOG_LBP.width/8,dsize_HOG_LBP.height/8), 9);
		cv::vector< float> descriptorsValues_vector;
		cv::vector< cv::Point> locations;
		d.compute(image_grayscale, descriptorsValues_vector, cv::Size(0,0), cv::Size(0,0), locations);
		cv::Mat descriptorsValues_temp(1, descriptorsValues_vector.size(),CV_32FC1);
		//convert descriptorsValues(type vector) to row matrix
		memcpy(descriptorsValues_temp.data,descriptorsValues_vector.data(), descriptorsValues_vector.size()*sizeof(float));
		descriptorsValues_temp.copyTo(descriptor_values);
	}
	else if(feature_number==2)
	{
		cv::Mat lbp_image;
		//resizing
		cv::resize(image, image_grayscale, dsize_HOG_LBP ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		//gray
		//cv::cvtColor(temp, image_grayscale, CV_RGB2GRAY);
		lbp::OLBP(image_grayscale,lbp_image);
		cv::normalize(lbp_image, lbp_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

		descriptor_values=lbp::spatial_histogram(lbp_image, 59, dsize_HOG_LBP.width/8, dsize_HOG_LBP.height/8, true);
	}
	else if (feature_number==3)
	{
		cv::Mat descriptorsValues_Mat;
		cv::resize(image, image_grayscale, dsize_BRIEF); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		cv::DenseFeatureDetector detector(12.0f, 1, 0.1f, 10);
		cv::BriefDescriptorExtractor extractor(32);
		std::vector<cv::KeyPoint> keyPoints;

		//gray
		//cvtColor(image_grayscale, image_grayscale, CV_RGB2GRAY,CV_32FC1);
		cv::KeyPoint pt(float(image_grayscale.cols/2),float(image_grayscale.rows/2),1,-1, 0, 0, -1);
		keyPoints.push_back(pt);

	//	detector.detect(image_grayscale,keyPoints);
		extractor.compute(image_grayscale,keyPoints,descriptorsValues_Mat);
		std::vector<float> descriptorsValues_vector;

		descriptorsValues_vector.assign(descriptorsValues_Mat.datastart, descriptorsValues_Mat.dataend);

		cv::Mat descriptorsValues_temp(1, descriptorsValues_vector.size(),CV_32FC1);
		//convert descriptorsValues(type vector) to row matrix
		memcpy(descriptorsValues_temp.data,descriptorsValues_vector.data(), descriptorsValues_vector.size()*sizeof(float));
		descriptorsValues_temp.copyTo(descriptor_values);
	}

	return descriptor_values;
}

cv::Mat FeatureReprenstation::get_feature_descriptor_from_training_data
(std::vector<std::string> FoldersFullNames,int number_or_letter,int feature_number,int single_or_combination)
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
//		std::cout<<foldername_temp<<std::endl;
		ImageNames = load_folder_of_image(foldername_temp);

		unsigned int FileNum = ImageNames.size();
		for(unsigned int k=0; k< FileNum; ++k)
		{
			sprintf(FullFileName, "%s%d.png", cstr, k+1);
//			std::cout<<"FullFileName: "<<FullFileName<<std::endl;
			cv::Mat img = cv::imread(FullFileName);

			descriptorsValues=get_feature_descriptor(img,feature_number,single_or_combination);
//			std::cout << "descriptorsValues dimensions: " << descriptorsValues.cols << " width x " << descriptorsValues.rows << " height" << std::endl;
			trainData.push_back(descriptorsValues);
			if (number_or_letter == 1)
			{
				std::istringstream ( foldersNames ) >> Classes;
			}
			else
			{
				Classes = convertLettersToASCII(foldersNames,single_or_combination);
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
	trainClasses.col(0).copyTo(trainData_trainClasses.col(trainData.cols+trainClasses.cols-1));

	return trainData_trainClasses;//trainData_trainClasses contains trainData and trainClasses
}

int FeatureReprenstation::convertLettersToASCII(std::string letter,int single_or_combination)
{
	int class_number;
	if (single_or_combination==2)
	{
		char label1 = letter.at(0);
		char label2 = letter.at(1);
		int class_number_temp = (int(label1)-64)*10 + (int(label2)-64);
		class_number=class_number_temp-10+(int(label1)-65)*16;
	}
	else if (single_or_combination==1)
	{
		class_number=int(letter.at(0))-64;
	}
    return class_number;
}

std::string FeatureReprenstation::convertASCIIToLetters(int number,int single_or_combination)
{
	std::string letter_class;
	std::stringstream ss;
	if (single_or_combination==2)
	{
		int ASCII_1st_letter;
		int remainder = number%26;


		if (remainder !=0)
		{
			ASCII_1st_letter = floor(number/26)+65;
			ss<<char(ASCII_1st_letter)<<char(remainder+64);
			letter_class = ss.str();
			ss.str("");
		}
		else
		{
			ASCII_1st_letter = floor(number/26)+64;

			ss<<char(ASCII_1st_letter)<<char(26+64);
			letter_class = ss.str();
			ss.str("");
		}
	}
	else if (single_or_combination==1)
	{
		ss<<char(number+64);
		letter_class = ss.str();
		ss.str("");
	}
    return letter_class;
}

cv::Mat FeatureReprenstation::load_all_training_data_with_feature_descriptors
(std::string training_path,int number_or_letter,int feature_number, int load, int single_or_combination)
{
	//load: 1. load data from yml. 0. load from raw data
//	std::cout<<"load:"<<load<<"    feature number: "<<feature_number<<std::endl;
//	std::cout << "type:"<<typeid(feature_number).name() << std::endl;
	cv::Mat trainData_and_trainClasses;
	std::string prefix,suffix;
	std::stringstream ss_numbers,ss_letters;
	std::string path_letters,path_numbers;

	std::size_t pos = training_path.find("TrainingDataRAW/");      // position of "live" in str
	std::string raw_path = training_path.substr (0,pos);

	if (single_or_combination==2)
	{
		ss_numbers<<training_path<<"numbers"<<"/";
		path_numbers = ss_numbers.str();
		ss_numbers.str("");
		std::cout<<"path_numbers: "<<path_numbers<<std::endl;
		ss_letters<<training_path<<"letters"<<"/";
		path_letters = ss_letters.str();
		ss_letters.str("");
	}
	else if (single_or_combination==1)
	{
		ss_numbers<<training_path<<"single_number"<<"/";
		path_numbers = ss_numbers.str();
		ss_numbers.str("");

		ss_letters<<training_path<<"single_letter"<<"/";
		path_letters = ss_letters.str();
		ss_letters.str("");
	}
	std::vector<std::string> TrainingFoldersFullNames;
	if (number_or_letter==1)
	{
		TrainingFoldersFullNames = folder_list(path_numbers);
		if (single_or_combination==2)
		{
			prefix = "numbers";
		}
		else if (single_or_combination==1)
		{
			prefix = "single_number";
		}
	}
	else if (number_or_letter==0)
	{
		TrainingFoldersFullNames = folder_list(path_letters);
		if (single_or_combination==2)
		{
			prefix = "letters";
		}
		else if (single_or_combination==1)
		{
			prefix = "single_letter";
		}
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

	std::stringstream ss;
	std::string yml_filename,yml_dataname;
	ss<<raw_path<<"TrainingDataYML/"<<prefix<<"_trainData_and_trainClasses_"<<suffix<<".yml";
	yml_filename=ss.str();
	ss.str("");
	ss<<prefix<<"_trainData_and_trainClasses_"<<suffix;
	yml_dataname=ss.str();
	ss.str("");


	if (load==0)
	{
		//std::cout<<"computing trainData_and_trainClasses from training dataset..."<<std::endl;

		trainData_and_trainClasses=get_feature_descriptor_from_training_data
				(TrainingFoldersFullNames,number_or_letter,feature_number,single_or_combination);

		cv::FileStorage fs(yml_filename, cv::FileStorage::WRITE);
		fs << yml_dataname << trainData_and_trainClasses;


	}
	else if (load==1)
	{
		//std::cout<<"Reading trainData_and_trainClasses from feature description files."<<std::endl;

		cv::FileStorage fs(yml_filename, cv::FileStorage::READ);
		ss.str("");
		fs[yml_dataname] >> trainData_and_trainClasses;
		fs.release();
	}
	else
		std::cout<<"[Feature representation ERROR: ] wrong load number is given!"<<std::endl;

	return trainData_and_trainClasses;
	}

cv::Mat FeatureReprenstation::preprocess_text_tag(cv::Mat& tag_image, int feature_number, int single_or_combination)
{
	//single_or_combination: 1-single, 2- combination
	std::vector<cv::Rect> image_portions;
	cv::Mat descriptor_values;
	//std::cout<<"single_or_combination: "<<single_or_combination<<std::endl;

	// 1. find the 4 combinations of two letters/numbers
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(0.8/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.7/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(11.0/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(21.2/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(31.5/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
//	for (double x_min_ratio = 0; x_min_ratio < 1.0; x_min_ratio += 0.25)
//	{
//		cv::Mat portion = tag_image(cv::Rect(tag_image.cols*x_min_ratio,0, tag_image.cols*0.25, tag_image.rows));
//		image_portions.push_back(portion);
//	}

	// for single letters: separate found image portions into two sections
	if (single_or_combination==1)
	{
		std::vector<cv::Rect> temp;
		for (size_t i=0; i<image_portions.size(); ++i)
		{
			temp.push_back(remove_white_image_border(tag_image, cv::Rect(image_portions[i].x, image_portions[i].y, image_portions[i].width/2, image_portions[i].height)));
			temp.push_back(remove_white_image_border(tag_image, cv::Rect(image_portions[i].x+image_portions[i].width/2, image_portions[i].y, image_portions[i].width/2, image_portions[i].height)));
		}
		image_portions = temp;
	}
//	if (single_or_combination==1)
//	{
//		// single letters/numbers
//		// todo: update numbers
//		double x1,x2,x3,x4,x5,x6,x7,x8,y,width,height;
//		x1=tag_image.cols*4.5/317.;
//		x2=tag_image.cols*33./317.;
//		x3=tag_image.cols*88./317.;
//		x4=tag_image.cols*118./317.;
//		x5=tag_image.cols*170./317.;
//		x6=tag_image.cols*200./317.;
//		x7=tag_image.cols*253./317.;
//		x8=tag_image.cols*281./317.;
//		y=tag_image.rows*5./59.;
//		height=tag_image.rows*100./129.;
//		width=tag_image.cols*62./690.;
//		cv::Mat letter1(tag_image, cv::Rect(x1,y,width,height));
//		image_portions.push_back(letter1);
//		cv::Mat letter2(tag_image, cv::Rect(x2,y,width,height));
//		image_portions.push_back(letter2);
//		cv::Mat number1(tag_image, cv::Rect(x3,y,width,height));
//		image_portions.push_back(number1);
//		cv::Mat number2(tag_image, cv::Rect(x4,y,width,height));
//		image_portions.push_back(number2);
//		cv::Mat number3(tag_image, cv::Rect(x5,y,width,height));
//		image_portions.push_back(number3);
//		cv::Mat number4(tag_image, cv::Rect(x6,y,width,height));
//		image_portions.push_back(number4);
//		cv::Mat number5(tag_image, cv::Rect(x7,y,width,height));
//		image_portions.push_back(number5);
//		cv::Mat number6(tag_image, cv::Rect(x8,y,width,height));
//		image_portions.push_back(number6);
//	}
//	else
//		std::cout<<"please give: 1 - single character classifier \n" <<	"                2 - combination classifier "<<std::endl;

	for (unsigned int i = 0; i<image_portions.size();i++)
		descriptor_values.push_back(get_feature_descriptor(tag_image(image_portions[i]), feature_number, single_or_combination));

	return descriptor_values;
}

void FeatureReprenstation::load_or_train_SVM_classifiers(cv::SVM& numbers_svm,cv::SVM& letters_svm,
															int load,int classifier,int feature_number,int single_or_combination,
															std::string path_data)
{
//	std::cout<<"package path: "<<package_path<<std::endl;
	double start_time,time_in_seconds;
	std::string suffix,classifier_name;
	cv::Mat numbers_trainData,letters_trainData,numbers_trainClasses,letters_trainClasses,
	numbers_trainData_and_trainClasses,letters_trainData_and_trainClasses;
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
	else
		std::cout<<"[Feature representation ERROR: ] wrong feature number is given!"<<std::endl;

	if (classifier==2)
		{

			//std::cout<<"start processing training data..."<<std::endl;
			std::string training_path = path_data + "TrainingDataRAW/";
//				std::string training_path = "/home/rmb-om/training_samples_for_SVM/";

			numbers_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors
														(training_path,1,feature_number,load,single_or_combination);
			letters_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors
														(training_path,0,feature_number,load,single_or_combination);

			cv::Mat numbers_trainData_temp (numbers_trainData_and_trainClasses,cv::Rect(0,0,numbers_trainData_and_trainClasses.cols-1,numbers_trainData_and_trainClasses.rows));
			cv::Mat letters_trainData_temp (letters_trainData_and_trainClasses,cv::Rect(0,0,letters_trainData_and_trainClasses.cols-1,letters_trainData_and_trainClasses.rows));
			numbers_trainData_temp.copyTo(numbers_trainData);
			letters_trainData_temp.copyTo(letters_trainData);
			numbers_trainClasses = numbers_trainData_and_trainClasses.col(numbers_trainData_and_trainClasses.cols-1);
			letters_trainClasses = letters_trainData_and_trainClasses.col(letters_trainData_and_trainClasses.cols-1);
		}

	//std::cout<<"finish processing training data..."<<std::endl;

	if (classifier==3 || classifier==2)
		{

			std::stringstream ss;
			std::string number_svm_model,letter_svm_model;
			classifier_name="SVM";
			std::cout<<"classifiers name: "<<classifier_name<<std::endl;
			char *cstr_number,*cstr_letter;
			if (single_or_combination==2)
			{
				ss<<path_data<<"TrainedSVMClassifiers/numbers_svm_model_"<<suffix<<".xml";
			}
			else if (single_or_combination==1)
			{
				ss<<path_data<<"TrainedSVMClassifiers/single_number_svm_model_"<<suffix<<".xml";
			}
			number_svm_model = ss.str();
			ss.str("");
			if (single_or_combination==2)
			{
				ss<<path_data<<"TrainedSVMClassifiers/letters_svm_model_"<<suffix<<".xml";
			}
			else if (single_or_combination==1)
			{
				ss<<path_data<<"TrainedSVMClassifiers/single_letter_svm_model_"<<suffix<<".xml";
			}
			letter_svm_model = ss.str();
			ss.str("");

			cstr_number = new char[number_svm_model.length() + 1];
			cstr_letter = new char[letter_svm_model.length() + 1];
			strcpy(cstr_number, number_svm_model.c_str());
			strcpy(cstr_letter, letter_svm_model.c_str());
			std::cout<<"letter_svm_model: "<<cstr_letter<<std::endl;
			std::cout<<"number_svm_model: "<<cstr_number<<std::endl;

			if(classifier==3)
			{
				//std::cout<<"loading SVM classifiers"<<std::endl;
				start_time=clock();
				std::cout<<"loading SVM classifiers"<<std::endl;

				numbers_svm.load(cstr_number);
				letters_svm.load(cstr_letter);
				time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
				std::cout<<"SVM classifiers loaded"<<std::endl;
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

				numbers_svm.train(numbers_trainData,numbers_trainClasses, cv::Mat(), cv::Mat(),params);
				letters_svm.train(letters_trainData,letters_trainClasses, cv::Mat(), cv::Mat(),params);

				time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
				//std::cout << "[" << time_in_seconds << " s] processing time for training SVM" << std::endl;

				numbers_svm.save(cstr_number);
				letters_svm.save(cstr_letter);

				//std::cout << "SVM training model Saved" << std::endl;
			}
			delete [] cstr_number;
			delete [] cstr_letter;
		}
}

void FeatureReprenstation::load_or_train_KNN_classifiers(cv::KNearest& numbers_knn,cv::KNearest& letters_knn,
															int load,int classifier,int feature_number,int single_or_combination,
															std::string path_data)
{
	double start_time,time_in_seconds;
	std::string classifier_name;
	cv::Mat numbers_trainData,letters_trainData,numbers_trainClasses,letters_trainClasses,
		numbers_trainData_and_trainClasses,letters_trainData_and_trainClasses;

	classifier_name="KNN";

	//std::cout<<"start processing training data..."<<std::endl;
	std::string training_path = path_data + "TrainingDataRAW/";
//	std::cout<<"training_path: "<<training_path<<std::endl;
//				std::string training_path = "/home/rmb-om/training_samples_for_SVM/";

	numbers_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors
												(training_path,1,feature_number,load,single_or_combination);
	letters_trainData_and_trainClasses = load_all_training_data_with_feature_descriptors
												(training_path,0,feature_number,load,single_or_combination);

	cv::Mat numbers_trainData_temp (numbers_trainData_and_trainClasses,cv::Rect(0,0,numbers_trainData_and_trainClasses.cols-1,numbers_trainData_and_trainClasses.rows));
	cv::Mat letters_trainData_temp (letters_trainData_and_trainClasses,cv::Rect(0,0,letters_trainData_and_trainClasses.cols-1,letters_trainData_and_trainClasses.rows));
	numbers_trainData_temp.copyTo(numbers_trainData);
	letters_trainData_temp.copyTo(letters_trainData);
	numbers_trainClasses = numbers_trainData_and_trainClasses.col(numbers_trainData_and_trainClasses.cols-1);
	letters_trainClasses = letters_trainData_and_trainClasses.col(letters_trainData_and_trainClasses.cols-1);

	//Build KNNs

//	cv::KNearest numbers_knn(numbers_trainData,numbers_trainClasses);
//	cv::KNearest letters_knn(letters_trainData,letters_trainClasses);

	//Start training

	//std::cout<<"training KNN classifiers with"<<std::endl;
	start_time = clock();
	numbers_knn.train(numbers_trainData, numbers_trainClasses);
	letters_knn.train(letters_trainData, letters_trainClasses);
	time_in_seconds = (clock() - start_time) / (double)CLOCKS_PER_SEC;
		//std::cout << "[" << time_in_seconds << " s] processing time for training KNN" << std::endl;

}


std::string FeatureReprenstation::read_text_tag_SVM(cv::SVM& numbers_svm, cv::SVM& letters_svm, cv::Mat& tag_image, int feature_number, int single_or_combination)
{
	double start_time,time_in_seconds;
	std::vector<int> text_label_result_int;
	std::string suffix;

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
	else
		std::cout<<"[Feature representation ERROR: ] wrong feature number is given!"<<std::endl;

	cv::Mat descriptors_values = preprocess_text_tag(tag_image,feature_number,single_or_combination);

	if (single_or_combination==2)
	{
		float response = letters_svm.predict(descriptors_values.row(0),descriptors_values.row(0).cols);
		text_label_result_int.push_back(response);

		for (unsigned int j =1;j<4;j++)
		{
			float response = numbers_svm.predict(descriptors_values.row(j),descriptors_values.row(j).cols);
			text_label_result_int.push_back(response);
		}
	}
	else if (single_or_combination==1)
	{
		for (unsigned int j =0;j<2;j++)
		{
			float response = letters_svm.predict(descriptors_values.row(j),descriptors_values.row(j).cols);
			text_label_result_int.push_back(response);
		}
		for (unsigned int j =2;j<8;j++)
		{
			float response = numbers_svm.predict(descriptors_values.row(j),descriptors_values.row(j).cols);
			text_label_result_int.push_back(response);
		}
	}

	std::stringstream ss;
	std::string text_label;
	if (single_or_combination==1)
	{
		ss<<convertASCIIToLetters(text_label_result_int[0],single_or_combination)
		<<convertASCIIToLetters(text_label_result_int[1],single_or_combination)
		<<"-"<<text_label_result_int[2]<<text_label_result_int[3]
		<<"-"<<text_label_result_int[4]<<text_label_result_int[5]
		<<"-"<<text_label_result_int[6]<<text_label_result_int[7];
		text_label = ss.str();
		ss.str("");
	}
	else if (single_or_combination==2)
	{
		ss<<convertASCIIToLetters(text_label_result_int[0],single_or_combination)<<"-"<<text_label_result_int[1]
							<<"-"<<text_label_result_int[2]<<"-"<<text_label_result_int[3];
		text_label = ss.str();
		ss.str("");
	}
	std::cout << "From $"<<"SVM"<<"$ with the feature $"<< suffix <<"$ got label = "<<text_label<< std::endl;

	return text_label;
}

std::string FeatureReprenstation::read_text_tag_KNN(cv::KNearest& numbers_knn,cv::KNearest& letters_knn,cv::Mat& testImg,int feature_number,int single_or_combination)
{
	double start_time,time_in_seconds;
	std::vector<int> text_label_result_int;
	std::string suffix;

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
	else
		std::cout<<"[Feature representation ERROR: ] wrong feature number is given!"<<std::endl;

	cv::Mat test_descriptorsValues=preprocess_text_tag(testImg,feature_number,single_or_combination);

	cv::Mat results(1,1,CV_32FC1);
	cv::Mat neighbourResponses = cv::Mat::ones(1,10,CV_32FC1);
	cv::Mat dist = cv::Mat::ones(1, 10, CV_32FC1);

	if (single_or_combination==2)
	{
		letters_knn.find_nearest(test_descriptorsValues.row(0),1,results, neighbourResponses, dist);
		text_label_result_int.push_back(results.at<float>(0,0));

		for (unsigned int j =1;j<4;j++)
		{
			numbers_knn.find_nearest(test_descriptorsValues.row(j),1,results, neighbourResponses, dist);
			text_label_result_int.push_back(results.at<float>(0,0));
		}
	}
	else if (single_or_combination==1)
	{
		for (unsigned int j =0;j<2;j++)
		{
			letters_knn.find_nearest(test_descriptorsValues.row(j),1,results, neighbourResponses, dist);
			text_label_result_int.push_back(results.at<float>(0,0));
		}
		for (unsigned int j =2;j<8;j++)
		{
			numbers_knn.find_nearest(test_descriptorsValues.row(j),1,results, neighbourResponses, dist);
			text_label_result_int.push_back(results.at<float>(0,0));
		}
	}


	std::stringstream ss;
	std::string text_label;
	if (single_or_combination==1)
	{
		ss<<convertASCIIToLetters(text_label_result_int[0],single_or_combination)
		<<convertASCIIToLetters(text_label_result_int[1],single_or_combination)
		<<"-"<<text_label_result_int[2]<<text_label_result_int[3]
		<<"-"<<text_label_result_int[4]<<text_label_result_int[5]
		<<"-"<<text_label_result_int[6]<<text_label_result_int[7];
		text_label = ss.str();
		ss.str("");
	}
	else if (single_or_combination==2)
	{
		ss<<convertASCIIToLetters(text_label_result_int[0],single_or_combination)<<"-"<<text_label_result_int[1]
							<<"-"<<text_label_result_int[2]<<"-"<<text_label_result_int[3];
		text_label = ss.str();
		ss.str("");
	}

	std::cout << "From $"<<"KNN"<<"$ with the feature $"<<suffix<<"$ got label = "<<text_label<< std::endl;

	return text_label;
}

void FeatureReprenstation::help()
{
 std::cout <<
         "Usage:\n"
         "./feature_representation_node \n"
         "string <image_name> \n"
         "int<load:1-load training data from file 0-load traning data from raw images> \n"
         "int<classifier: 1-KNN,2-train svm, 3 load svm>\n "
         "int<feature number:1. HOG 2. LBP. 3 BRIEF>\n"
		 "int<single or combinations :1. single 2. combinations>"<< std::endl;
 std::cout << "---------------------------------------------------------------------------------------"<<std::endl;
}

