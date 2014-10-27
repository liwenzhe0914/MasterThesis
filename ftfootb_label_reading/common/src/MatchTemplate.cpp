#include "ftfootb_label_reading/MatchTemplate.h"


MatchTemplate::MatchTemplate(const std::string& templates_storage_path)
{
	// todo: set parameters
	standard_tag_image_height_ = 38.;

	// load templates for letter pairs and number pairs
	std::string path = templates_storage_path;
	if (path.at(path.length() - 1) != '/')
		path.append("/");
	std::string path_letters = path + "LetterTemplates/";
	std::string path_numbers = path + "NumberTemplates/";
	load_templates(path_letters, letter_pair_templates);
	load_templates(path_numbers, number_pair_templates);

	// resize templates to a desired scale
	resize_templates(letter_pair_templates);
	resize_templates(number_pair_templates);
}

void MatchTemplate::load_templates(const std::string& templates_storage_path, TokenTemplates& token_templates)
{
//	std::string arg(argv[2]);
//	std::string imgpath = arg.substr(0, arg.find_last_of("/") + 1);

	/// read all the template image files from input folder
	std::vector<std::string> templateImageNames;
	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir(templates_storage_path.c_str())) != NULL)
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
					std::string s = templates_storage_path;
					if (s.at(s.length() - 1) != '/')
						s.append("/");
					s.append(entry->d_name);
					templateImageNames.push_back(s);
					//std::cout << "imagename: " << s << std::endl;
				}
			}
		}
	}
	else
	{
		std::cout << "MatchTemplate::load_templates: Error: Could not open path." << std::endl;
		return;
	}

	///build a new map between template image raw name and template images
	for (unsigned int i = 0; i < templateImageNames.size(); i++)
	{
		int lastindex = templateImageNames[i].find_last_of(".");
		int lastindex2 = templateImageNames[i].find_last_of("/");
		std::string rawname = templateImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
		token_templates[rawname] = cv::imread(templateImageNames[i], CV_LOAD_IMAGE_GRAYSCALE);			// todo: check whether color image is needed
	}

	std::cout << "Template images loaded: " << templateImageNames.size() << std::endl;
}


void MatchTemplate::resize_templates(TokenTemplates& token_templates)
{
	// todo: make this a parameter
	double letter_height_tag_image_height_ratio = 16./38.;		// 15./23.

	for (TokenTemplatesIterator it=token_templates.begin(); it!=token_templates.end(); ++it)
	{
		/// resize template accordingly
		//std::cout << "Current template:" << it->first << std::endl;
		cv::Mat template_original = it->second;
		double resize_factor = letter_height_tag_image_height_ratio*standard_tag_image_height_ / template_original.rows;
		//std::cout << "resize_factor:" << resize_factor << std::endl;
		cv::Mat template_resized;
		cv::resize(template_original, template_resized, cv::Size(), resize_factor, resize_factor, cv::INTER_AREA); //resize image
		//std::cout << "resized template size: " << template_resized.rows <<"x"<<template_resized.cols<< std::endl;
		it->second = template_resized;
	}
}


void MatchTemplate::read_tag(const cv::Mat& tag_image, std::string& tag_label)
{
	// todo: make this a parameter
	double token_threshold = 0.4;

	tag_label.clear();
	std::multimap<double, std::string> matching_scores;

	// 1. read letter pair
	cv::Mat tag_roi = tag_image(cv::Rect(0,0, tag_image.cols*0.25, tag_image.rows));
	match_token_templates(tag_roi, letter_pair_templates, matching_scores);
	if (matching_scores.begin()->first < token_threshold)
		tag_label.append(matching_scores.begin()->second);
	else
		tag_label.append("**");

//	int i=0;
//	for (std::multimap<double, std::string>::iterator it=matching_scores.begin(); it!=matching_scores.end() && i<10; ++it, ++i)
//		std::cout << it->first << ": " << it->second << std::endl;
//	std::cout << std::endl;

	// 2. read three segments of number pairs
	for (double x_min_ratio = 0.25; x_min_ratio < 1.0; x_min_ratio += 0.25)
	{
		cv::Mat tag_roi = tag_image(cv::Rect(tag_image.cols*x_min_ratio,0, tag_image.cols*0.25, tag_image.rows));
		match_token_templates(tag_roi, number_pair_templates, matching_scores);
		tag_label.append("-");
		if (matching_scores.begin()->first < token_threshold)
			tag_label.append(matching_scores.begin()->second);
		else
			tag_label.append("**");

//		int i=0;
//		for (std::multimap<double, std::string>::iterator it=matching_scores.begin(); it!=matching_scores.end() && i<10; ++it, ++i)
//			std::cout << it->first << ": " << it->second << std::endl;
//		std::cout << std::endl;
	}
}


void MatchTemplate::match_token_templates(const cv::Mat& image, const TokenTemplates& token_templates, std::multimap<double, std::string>& matching_scores)
{
	// todo: make this a parameter
	int match_method = cv::TM_CCOEFF_NORMED;
//	double letter_height_tag_image_height_ratio = 16./38.;		// 15./23.

	// binarization
	cv::Mat image_binarized;
	cv::adaptiveThreshold(image, image_binarized, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, -5);
//	cv::imshow("bin_image", image_binarized);

	// resize image
	cv::Mat image_resized = image_binarized;
	double resize_factor = standard_tag_image_height_/(double)image.rows;
	if (resize_factor != 1.0)
		cv::resize(image_binarized, image_resized, cv::Size(), resize_factor, resize_factor, cv::INTER_AREA);
//	cv::imshow("tag_image", image_resized);
//	cv::waitKey();

	matching_scores.clear();
	for (TokenTemplates::const_iterator it=token_templates.begin(); it!=token_templates.end(); ++it)
	{
//		/// resize template accordingly
//		//std::cout << "Current template:" << it->first << std::endl;
//		cv::Mat template_original = it->second;
//		double resize_factor = letter_height_tag_image_height_ratio*image.rows / template_original.rows;
//		//std::cout << "resize_factor:" << resize_factor << std::endl;
//		cv::Mat template_resized;
//		cv::resize(template_original, template_resized, cv::Size(), resize_factor, resize_factor, cv::INTER_AREA); //resize image
//		//std::cout << "resized template size: " << template_resized.rows <<"x"<<template_resized.cols<< std::endl;
		cv::Mat template_resized = it->second;

		/// do the Matching
		//std::cout << "roi size: " << image.rows - template_resized.rows + 1 << "x" << image.cols - template_resized.cols + 1 << std::endl;
		cv::Mat match_matrix(image_resized.rows - template_resized.rows + 1, image_resized.cols - template_resized.cols + 1, CV_32FC1);
		cv::matchTemplate(image_resized, template_resized, match_matrix, match_method);
		//cv::imshow("region of interest", image);
		//cv::waitKey();

		/// determine the best matching score
		double minVal, maxVal, score = 0;
		cv::Point minLoc, maxLoc;
		cv::Point matchLoc;
		cv::minMaxLoc(match_matrix, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better.
		if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
		{
			matchLoc = minLoc;
			score=minVal;
		}
		else
		{
			matchLoc = maxLoc;
			score= 1.-maxVal;
		}

		/// store the score
		matching_scores.insert(std::pair<double, std::string>(score, it->first));
	}
}
