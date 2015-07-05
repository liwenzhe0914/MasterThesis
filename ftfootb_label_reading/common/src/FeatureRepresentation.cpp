#include "ftfootb_label_reading/FeatureRepresentation.h"
#include "ftfootb_label_reading/timer.h"

#include <fstream>


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

cv::Mat FeatureRepresentation::wolf_thresholding(cv::Mat& img_gray)
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

void FeatureRepresentation::get_folder_list(std::string path, std::vector<std::string>& folder_list)
{
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(path.c_str())) != NULL)
	{
		// print all the files and directories within directory
		while ((entry = readdir(pDIR)) != NULL)
		{
			std::string completeName = entry->d_name;
			std::string name_end = completeName.substr(completeName.find_last_of("/") + 1, completeName.length());
			std::string s = path;
			if (name_end !="." and name_end !="..")
			{
				s.append(completeName);
				folder_list.push_back(s);
			}
		}
	}
	else
		std::cout << "Error: Could not open path." << std::endl;
}

void FeatureRepresentation::get_image_file_list(std::string path, std::vector<std::string>& image_file_list)
{
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
					image_file_list.push_back(s);
				}
			}
		}
	}
	else
		std::cout << "Error: Could not open path!" << std::endl;
}


cv::Rect FeatureRepresentation::remove_white_image_border(const cv::Mat& image, cv::Rect roi)
{
	// remove white border
	cv::Mat binary_image, temp;
	if (roi.x + roi.width > image.cols)
		roi.width = image.cols - roi.x;
	if (roi.y + roi.height > image.rows)
		roi.height = image.rows - roi.y;
	cv::threshold(image(roi), binary_image, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
//	cv::dilate(binary_image, temp, cv::Mat(), cv::Point(-1,-1), 1);
//	cv::erode(temp, binary_image, cv::Mat(), cv::Point(-1,-1), 1);
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
	cv::Rect bounding_box(roi.x + min_point.x, roi.y + min_point.y, std::max(std::min(max_point.x-min_point.x+1, roi.width), 1), std::max(std::min(max_point.y-min_point.y+1, roi.height), 1));

//	cv::imshow("padding", image);
//	cv::imshow("binary", binary_image);
//	cv::Mat cut = image(bounding_box);
//	cv::imshow("cut", cut);
//	cv::waitKey();

	return bounding_box;
}


cv::Mat FeatureRepresentation::get_feature_descriptor(const cv::Mat& image, int feature_type, int single_or_combination)
{
	//read image file
	cv::Mat descriptor;
	cv::Mat image_grayscale;
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

	if(feature_type==1)
	{
		//resizing
		cv::resize(image, image_grayscale, dsize_HOG_LBP); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );

		cv::HOGDescriptor d( dsize_HOG_LBP, cv::Size(dsize_HOG_LBP.width/4,dsize_HOG_LBP.height/4),
				cv::Size(dsize_HOG_LBP.width/8,dsize_HOG_LBP.height/8), cv::Size(dsize_HOG_LBP.width/8,dsize_HOG_LBP.height/8), 9);
		cv::vector<float> descriptor_values_vector;
		cv::vector<cv::Point> locations;
		d.compute(image_grayscale, descriptor_values_vector, cv::Size(0,0), cv::Size(0,0), locations);
		descriptor.create(1, descriptor_values_vector.size(), CV_32FC1);
		memcpy(descriptor.data, descriptor_values_vector.data(), descriptor_values_vector.size()*sizeof(float));
//		cv::Mat descriptorsValues_temp(1, descriptor_values_vector.size(),CV_32FC1);
		//convert descriptorsValues(type vector) to row matrix
//		memcpy(descriptorsValues_temp.data,descriptor_values_vector.data(), descriptor_values_vector.size()*sizeof(float));
//		descriptorsValues_temp.copyTo(descriptor);
	}
	else if(feature_type==2)
	{
		cv::Mat lbp_image;
		//resizing
		cv::resize(image, image_grayscale, dsize_HOG_LBP ); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		//gray
		//cv::cvtColor(temp, image_grayscale, CV_RGB2GRAY);
		lbp::OLBP(image_grayscale,lbp_image);
		cv::normalize(lbp_image, lbp_image, 0, 255, cv::NORM_MINMAX, CV_8UC1);

		descriptor=lbp::spatial_histogram(lbp_image, 59, dsize_HOG_LBP.width/8, dsize_HOG_LBP.height/8, true);
	}
	else if (feature_type==3)
	{
//		cv::Mat descriptor_values_mat;
		cv::resize(image, image_grayscale, dsize_BRIEF); //Size(64,48) ); //Size(32*2,16*2)); //Size(80,72) );
		cv::DenseFeatureDetector detector(12.0f, 1, 0.1f, 10);
		cv::BriefDescriptorExtractor extractor(32);
		std::vector<cv::KeyPoint> keyPoints;

		//gray
		//cvtColor(image_grayscale, image_grayscale, CV_RGB2GRAY,CV_32FC1);
		cv::KeyPoint pt(float(image_grayscale.cols/2),float(image_grayscale.rows/2),1,-1, 0, 0, -1);
		keyPoints.push_back(pt);

//		detector.detect(image_grayscale,keyPoints);
		extractor.compute(image_grayscale, keyPoints, descriptor);
//		extractor.compute(image_grayscale,keyPoints, descriptor_values_mat);
//		std::vector<float> descriptor_values_vector;
//		descriptor_values_vector.assign(descriptor_values_mat.datastart, descriptor_values_mat.dataend);
//
//		cv::Mat descriptorsValues_temp(1, descriptor_values_vector.size(),CV_32FC1);
//		//convert descriptorsValues(type vector) to row matrix
//		memcpy(descriptorsValues_temp.data,descriptor_values_vector.data(), descriptor_values_vector.size()*sizeof(float));
//		descriptorsValues_temp.copyTo(descriptor);
	}

	return descriptor;
}

void FeatureRepresentation::get_feature_descriptor_from_training_data(const std::string path_training_data_files, int number_or_letter,
		int feature_type, int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels)
{
	// get list of data folders (each folder contains images for one class)
	std::vector<std::string> training_folder_list;
	get_folder_list(path_training_data_files, training_folder_list);

	// iterate over classes
	for(size_t i=0; i<training_folder_list.size(); ++i)
	{
		// compute class label
		const std::string folder_name = training_folder_list[i].substr(training_folder_list[i].find_last_of("/") + 1, training_folder_list[i].length());
		int class_label = -1;
		if (number_or_letter == 1)
			std::istringstream(folder_name) >> class_label;
		else
			class_label = convertLettersToASCII(folder_name, single_or_combination);

		// open each image of this class
		std::vector<std::string> image_file_list;
		get_image_file_list(training_folder_list[i], image_file_list);
		for(size_t k=0; k<image_file_list.size(); ++k)
		{
			// compute descriptor
//			std::cout << "image_file_list[k]: " << image_file_list[k] << std::endl;
			cv::Mat img = cv::imread(image_file_list[k], CV_LOAD_IMAGE_GRAYSCALE);
			cv::Rect roi = remove_white_image_border(img, cv::Rect(0,0,img.cols, img.rows));
			cv::Mat descriptor = get_feature_descriptor(img(roi), feature_type, single_or_combination);
			train_data.push_back(descriptor);

			// label
			train_labels.push_back(class_label);
		}
	}
}

int FeatureRepresentation::convertLettersToASCII(const std::string letter, const int single_or_combination)
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

std::string FeatureRepresentation::convertASCIIToLetters(int number,int single_or_combination)
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

void FeatureRepresentation::compute_descriptor_pyramid(const cv::Mat& image, const int label, int feature_type, cv::Mat& train_data, cv::Mat& train_labels)
{
	const int min_height = 15;
	const int single_or_combination = 1;

	// vary brightness range
	for (double dynamics_factor=1.0; dynamics_factor>0.1; dynamics_factor-=0.2)
	{
		for (int offset=0; offset<=255*(1.-dynamics_factor); offset += 10)
		{
			cv::Mat dynamics_image = image*dynamics_factor + offset;

//			cv::imshow("sample", dynamics_image);
//			cv::waitKey();

			// sizes
			cv::Size target_size(image.cols, image.rows);
			while (target_size.height >= min_height)
			{
				// resize image, compute descriptor and store to descriptor matrix
				cv::Mat image_resized;
				cv::resize(dynamics_image, image_resized, target_size);
				cv::Rect cropped_roi = remove_white_image_border(image_resized, cv::Rect(0, 0, image_resized.cols, image_resized.rows));
				train_data.push_back(get_feature_descriptor(image_resized(cropped_roi), feature_type, single_or_combination));
				train_labels.push_back(label);

				// new target size
				target_size.width *= 0.8;
				target_size.height *= 0.8;

//				cv::imshow("sample", image_resized(cropped_roi));
//				cv::waitKey();
			}
		}
	}
}

void FeatureRepresentation::load_symbol_templates_and_generate_training_data(std::string template_path, int letter_or_number,
		int feature_type, cv::Mat& train_data, cv::Mat& train_labels)
{
	const std::string symbol_type_str = (letter_or_number==0 ? "letters" : "numbers");
	const int single_or_combination = 1;

	// 1. load template
	const std::string txt_filename = template_path + symbol_type_str + ".txt";
	const std::string image_filename = template_path + symbol_type_str + ".png";
	// read txt file
	std::vector<int> labels;
	std::ifstream file(txt_filename.c_str(), std::ios::in);
	if (file.is_open()==true)
	{
		int number_symbols = 0;
		file >> number_symbols;
		for (int i=0; i<number_symbols; ++i)
		{
			if (letter_or_number == 0)
			{
				std::string symbol;
				file >> symbol;
				labels.push_back(convertLettersToASCII(symbol, single_or_combination));
			}
			else
			{
				int symbol;
				file >> symbol;
				labels.push_back(symbol);
			}
		}
	}
	else
	{
		std::cout << "Error: FeatureRepresentation::load_symbol_templates_and_generate_training_data: Could not open file " << txt_filename << std::endl;
		return;
	}
	// load image
	cv::Mat symbol_templates_image = cv::imread(image_filename, CV_LOAD_IMAGE_GRAYSCALE);

	// 2. separate symbols, generate standard template for each symbol
	std::vector<cv::Mat> symbol_templates(labels.size());
	const double symbol_width = symbol_templates_image.cols / (double)labels.size();
	const int cut_out_padding = 3;		// neglect so many pixels at borders when reading out symbols from symbol_templates_image
	const int base_template_padding = 10;	// add so many white pixels to the border of the cut out symbols for the base template
	for (size_t i=0; i<labels.size(); ++i)
	{
		cv::Rect cout_out_roi(i*symbol_width+cut_out_padding, cut_out_padding, symbol_width-2*cut_out_padding, symbol_templates_image.rows-2*cut_out_padding);
		cv::Rect symbol_roi_narrow = remove_white_image_border(symbol_templates_image, cout_out_roi);
		cv::Mat symbol_template = 255*cv::Mat::ones(symbol_roi_narrow.height+2*base_template_padding, symbol_roi_narrow.width+2*base_template_padding, symbol_templates_image.type());
		symbol_templates_image(symbol_roi_narrow).copyTo(symbol_template(cv::Rect(base_template_padding, base_template_padding, symbol_roi_narrow.width, symbol_roi_narrow.height)));
		symbol_templates[i] = symbol_template;
	}

	// 3. compute several image disturbances, sizes, etc. and compute descriptor
	for (size_t i=0; i<labels.size(); ++i)
	{
		// a. as is
		compute_descriptor_pyramid(symbol_templates[i], labels[i], feature_type, train_data, train_labels);

		// b. Gaussian blur
		for (int k=3; k<9; k+=2)
		{
			cv::Mat img;
			cv::GaussianBlur(symbol_templates[i], img, cv::Size(k,k), 0, 0);
			compute_descriptor_pyramid(img, labels[i], feature_type, train_data, train_labels);
		}

		// c. with dark pixels at border
		int draw_offset = base_template_padding*0.25;
		std::vector<cv::Point> line_points;
		line_points.push_back(cv::Point(draw_offset, symbol_templates[i].rows/4)); line_points.push_back(cv::Point(draw_offset, 3*symbol_templates[i].rows/4));
		line_points.push_back(cv::Point(symbol_templates[i].cols-draw_offset, symbol_templates[i].rows/4)); line_points.push_back(cv::Point(symbol_templates[i].cols-draw_offset, 3*symbol_templates[i].rows/4));
		line_points.push_back(cv::Point(symbol_templates[i].cols/4, draw_offset)); line_points.push_back(cv::Point(3*symbol_templates[i].cols/4, draw_offset));
		line_points.push_back(cv::Point(symbol_templates[i].cols/4, symbol_templates[i].rows-draw_offset)); line_points.push_back(cv::Point(3*symbol_templates[i].cols/4, symbol_templates[i].rows-draw_offset));
		draw_offset = base_template_padding*0.75;
		line_points.push_back(cv::Point(draw_offset, symbol_templates[i].rows/2)); line_points.push_back(cv::Point(draw_offset, symbol_templates[i].rows/2+5));
		line_points.push_back(cv::Point(symbol_templates[i].cols-draw_offset, symbol_templates[i].rows/2)); line_points.push_back(cv::Point(symbol_templates[i].cols-draw_offset, symbol_templates[i].rows/2+5));
		for (int j=0; j<(int)(line_points.size())/2; ++j)
		{
			cv::Mat img = symbol_templates[i].clone();
			cv::line(img, line_points[2*j], line_points[2*j+1], cv::Scalar(0), 2);
			compute_descriptor_pyramid(img, labels[i], feature_type, train_data, train_labels);
//			cv::imshow("sample", img);
//			cv::waitKey();
		}

		// d. random noise
		for (double noise_level=0.05; noise_level<=0.3; noise_level+=0.05)
		{
			cv::Mat img = symbol_templates[i].clone();
			for (int v=0; v<img.rows; ++v)
				for (int u=0; u<img.cols; ++u)
					img.at<uchar>(v,u) = std::max(0, std::min(255, (int)(img.at<uchar>(v,u)+(255.*noise_level*((double)rand()/(double)RAND_MAX - 0.5)))));
			compute_descriptor_pyramid(img, labels[i], feature_type, train_data, train_labels);
		}
	}
}

void FeatureRepresentation::load_all_training_data_with_feature_descriptors(std::string training_path, int letter_or_number,
		int feature_type, int training_data_source, int single_or_combination, cv::Mat& train_data, cv::Mat& train_labels)
{
	const std::string number_symbols_str = get_number_symbols_string(single_or_combination);
	const std::string feature_type_str = get_feature_type_string(feature_type);
	const std::string symbol_type_str = (letter_or_number==0 ? "letter" : "number");
	const std::string yml_filename = training_path + "yaml/" + number_symbols_str + "_" + symbol_type_str + "_training_data_" + feature_type_str + ".yml";
	const std::string yml_train_data = number_symbols_str + "_" + symbol_type_str + "_train_data_" + feature_type_str;
	const std::string yml_train_labels = number_symbols_str + "_" + symbol_type_str + "_train_labels_" + feature_type_str;

	if (training_data_source==1)
	{
		// load images from hard disk and compute descriptors, store descriptors to yaml file
		std::cout << "computing training data and labels from training dataset..." << std::endl;
		const std::string path_training_data_files = training_path + number_symbols_str + "_" + symbol_type_str + "/";
		get_feature_descriptor_from_training_data(path_training_data_files, letter_or_number, feature_type, single_or_combination, train_data, train_labels);

		cv::FileStorage fs(yml_filename, cv::FileStorage::WRITE);
		fs << yml_train_data << train_data;
		fs << yml_train_labels << train_labels;
	}
	else if (training_data_source==2)
	{
		// load descriptor data from yaml file
		std::cout << "Reading training data and labels from feature description file." << std::endl;
		cv::FileStorage fs(yml_filename, cv::FileStorage::READ);
		fs[yml_train_data] >> train_data;
		fs[yml_train_labels] >> train_labels;
		fs.release();
	}
	else
		std::cout<<"[Feature representation ERROR: ] wrong training_data_source number is given!"<<std::endl;
}

void FeatureRepresentation::load_training_data(std::string path_data, int feature_type, int training_data_source, int single_or_combination,
		cv::Mat& numbers_train_data, cv::Mat& numbers_train_labels, cv::Mat& letters_train_data, cv::Mat& letters_train_labels)
{
	std::cout << "loading training data ..." << std::endl;
	if (training_data_source == 0)
	{
		if (single_or_combination != 1)
		{
			std::cout << "Error: FeatureRepresentation::load_training_data: training with generated data requires to work with single letter/number format. Pairs of symbols are not supported. Please set single_or_combination to 1." << std::endl;
			return;
		}
		std::string template_path = path_data + "tag_template/";
		load_symbol_templates_and_generate_training_data(template_path, 0, feature_type, letters_train_data, letters_train_labels);
		load_symbol_templates_and_generate_training_data(template_path, 1, feature_type, numbers_train_data, numbers_train_labels);
	}
	else
	{
		std::string training_path = path_data + "training_data/";
		load_all_training_data_with_feature_descriptors(training_path, 0, feature_type, training_data_source, single_or_combination, letters_train_data, letters_train_labels);
		load_all_training_data_with_feature_descriptors(training_path, 1, feature_type, training_data_source, single_or_combination, numbers_train_data, numbers_train_labels);
	}
	std::cout << "training data loaded" << std::endl;
}


void FeatureRepresentation::load_or_train_SVM_classifiers(cv::SVM& numbers_svm, cv::SVM& letters_svm, int training_data_source,
		int classifier, int feature_type, int single_or_combination, std::string path_data)
{
	std::string number_symbols_str = get_number_symbols_string(single_or_combination);
	std::string feature_type_str = get_feature_type_string(feature_type);
	std::string number_svm_model = path_data + "trained_classifiers/" + number_symbols_str + "_number_svm_model_" + feature_type_str + ".xml";
	std::string letter_svm_model = path_data + "trained_classifiers/" + number_symbols_str + "_letter_svm_model_" + feature_type_str + ".xml";

	if (classifier==2)
	{
		// load training data
		cv::Mat numbers_train_data, letters_train_data, numbers_train_labels, letters_train_labels;
		load_training_data(path_data, feature_type, training_data_source, single_or_combination, numbers_train_data, numbers_train_labels, letters_train_data, letters_train_labels);

		// train SVM
		Timer tim;
		std::cout << "training SVM classifiers ..." << std::endl;
		//cv::SVMParams params(cv::SVM::C_SVC, cv::SVM::LINEAR, 0, 0, 0, 1, 0, 0, 0, cv::TermCriteria(CV_TERMCRIT_ITER, (int)1e7, 1e-6));
		cv::SVMParams params(cv::SVM::C_SVC, cv::SVM::RBF, 0, 0.01, 0, 1., 0, 0, 0, cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 1e7, FLT_EPSILON));	//(CV_TERMCRIT_ITER, (int)1e7, 1e-6);
		numbers_svm.train(numbers_train_data,numbers_train_labels, cv::Mat(), cv::Mat(),params);
		letters_svm.train(letters_train_data,letters_train_labels, cv::Mat(), cv::Mat(),params);
		std::cout << "[" << tim.getElapsedTimeInSec() << " s] processing time for training SVM" << std::endl;

		// store trained SVMs
		numbers_svm.save(number_svm_model.c_str());
		letters_svm.save(letter_svm_model.c_str());
		std::cout << "SVM training model saved" << std::endl;
	}
	else if(classifier==3)
	{
		// load trained classifiers from disk
		Timer tim;
		std::cout << "loading SVM classifiers ..." << std::endl;
		numbers_svm.load(number_svm_model.c_str());
		letters_svm.load(letter_svm_model.c_str());
		std::cout << "[" << tim.getElapsedTimeInSec() << " s] processing time for loading 2 SVMs." << std::endl;
	}
}

void FeatureRepresentation::load_or_train_KNN_classifiers(cv::KNearest& numbers_knn, cv::KNearest& letters_knn, int training_data_source,
		int classifier, int feature_type, int single_or_combination, std::string path_data)
{
	// load training data
	cv::Mat numbers_train_data, letters_train_data, numbers_train_labels, letters_train_labels;
	load_training_data(path_data, feature_type, training_data_source, single_or_combination, numbers_train_data, numbers_train_labels, letters_train_data, letters_train_labels);

	// train KNN classifiers
	Timer tim;
	std::cout << "training KNN classifiers" << std::endl;
	numbers_knn.train(numbers_train_data, numbers_train_labels);
	letters_knn.train(letters_train_data, letters_train_labels);
	std::cout << "[" << tim.getElapsedTimeInSec() << " s] processing time for training KNN" << std::endl;
}


cv::Mat FeatureRepresentation::get_descriptor_from_text_tag(const cv::Mat& tag_image, const int feature_number, const int single_or_combination)
{
	// single_or_combination: 1-single, 2- combination

	// 1. find the 4 combinations of two letters/numbers
	std::vector<cv::Rect> image_portions;
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(0.8/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.7/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(11.0/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(21.2/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));
	image_portions.push_back(remove_white_image_border(tag_image, cv::Rect(31.5/39.6*tag_image.cols, 0.9/7*tag_image.rows, 7.4/39.6*tag_image.cols, 5.4/7*tag_image.rows)));

	// 2. for single letters: separate found image portions into two sections
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

	// 3. compute descriptor
	cv::Mat descriptor_values;
	for (unsigned int i = 0; i<image_portions.size();i++)
		descriptor_values.push_back(get_feature_descriptor(tag_image(image_portions[i]), feature_number, single_or_combination));

	return descriptor_values;
}

std::string FeatureRepresentation::convert_classification_labels_to_string(const std::vector<int>& classification_labels, const int single_or_combination)
{
	std::stringstream text_label;
	const int number_symbols_per_block = (single_or_combination==1 ? 2 : 1);
	for (int j=0; j<number_symbols_per_block; ++j)
		text_label << convertASCIIToLetters(classification_labels[j], single_or_combination);
	for (int j=1; j<4; ++j)
	{
		text_label << "-";
		for (int k=0; k<number_symbols_per_block; ++k)
			text_label << classification_labels[j*number_symbols_per_block+k];
	}

	return text_label.str();
}

std::string FeatureRepresentation::read_text_tag_SVM(const cv::SVM& numbers_svm, const cv::SVM& letters_svm, const cv::Mat& tag_image, const int feature_type, const int single_or_combination)
{
	// compute descriptors
	cv::Mat descriptors = get_descriptor_from_text_tag(tag_image, feature_type, single_or_combination);

	// classify symbols
	std::vector<int> classification_labels;
	const int number_symbols_per_block = (single_or_combination==1 ? 2 : 1);
	for (int j=0; j<number_symbols_per_block; ++j)
		classification_labels.push_back(letters_svm.predict(descriptors.row(j)));	//, descriptors.row(j).cols);
	for (int j=number_symbols_per_block; j<4*number_symbols_per_block; ++j)
		classification_labels.push_back(numbers_svm.predict(descriptors.row(j)));	//,descriptors.row(j).cols);

	// convert to text
	std::string text_label = convert_classification_labels_to_string(classification_labels, single_or_combination);
	std::cout << "From $SVM$ with the feature $" << get_feature_type_string(feature_type) << "$ got label = " << text_label << std::endl;

	return text_label;
}

std::string FeatureRepresentation::read_text_tag_KNN(const cv::KNearest& numbers_knn, const cv::KNearest& letters_knn, const cv::Mat& tag_image, const int feature_type, const int single_or_combination)
{
	// compute descriptors
	cv::Mat descriptors = get_descriptor_from_text_tag(tag_image, feature_type, single_or_combination);

	// classify symbols
	std::vector<int> classification_labels;
	const int number_symbols_per_block = (single_or_combination==1 ? 2 : 1);
	for (int j=0; j<number_symbols_per_block; ++j)
		classification_labels.push_back(letters_knn.find_nearest(descriptors.row(j), 1));
	for (int j=number_symbols_per_block; j<4*number_symbols_per_block; ++j)
		classification_labels.push_back(numbers_knn.find_nearest(descriptors.row(j), 1));

	// convert to text
	std::string text_label = convert_classification_labels_to_string(classification_labels, single_or_combination);
	std::cout << "From $SVM$ with the feature $" << get_feature_type_string(feature_type) << "$ got label = " << text_label << std::endl;

	return text_label;
}

void FeatureRepresentation::help()
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

