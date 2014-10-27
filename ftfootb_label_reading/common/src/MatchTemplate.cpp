#include "ftfootb_label_reading/MatchTemplate.h"

int main( int argc, char** argv )
{
	std::string letters;
	double start_time;
	double time_in_seconds;
	start_time = clock();
	if (argc < 2)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}

	std::vector<std::string> allImageNames;
//	std::string arg(argv[2]);
//	std::string imgpath = arg.substr(0, arg.find_last_of("/") + 1);
	boost::filesystem::path input_path(argv[2]);
	img = cv::imread( argv[1], 1 );
	//std::cout << "source imagename: " << argv[1] << std::endl;
	std::cout << "image size: " << img.cols<<"x"<<img.rows<< std::endl;
	
	/// read all the image files from input folder
	
	if (boost::filesystem::is_directory(input_path))
	{
		DIR *pDIR;
		struct dirent *entry;
		if ((pDIR = opendir(argv[2])))
		while ((entry = readdir(pDIR)))
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
				std::string s = argv[2];
				if (s.at(s.length() - 1) != '/')
					s.append("/");
				s.append(entry->d_name);
				allImageNames.push_back(s);
				//std::cout << "imagename: " << s << std::endl;
			}
		}
	}
	else
		allImageNames.push_back(argv[2]); //single image to be processed

	std::cout << "Images to be processed: " << allImageNames.size() << std::endl;

	///build a new map between template image raw name and template images
	std::map<std::string, cv::Mat> letter_templates;
	for (unsigned int i = 0; i < allImageNames.size(); i++){
		int lastindex = allImageNames[i].find_last_of(".");
		int lastindex2 = allImageNames[i].find_last_of("/");
		std::string rawname = allImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
		letter_templates[rawname] = cv::imread(allImageNames[i],CV_LOAD_IMAGE_COLOR);
	}

//	double score_pre = -1e10;
	for (unsigned int i = 0; i < allImageNames.size(); i++)
	{	

		//std::cout << "current template imagename: " << allImageNames[i]<< std::endl;
		/// get image name without extension and path
		int lastindex = allImageNames[i].find_last_of(".");
		int lastindex2 = allImageNames[i].find_last_of("/");
		std::string rawname = allImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
		std::cout << "Current template:" << rawname << std::endl;
		templ_original = letter_templates[rawname]; // originalImage like it used to be, never modify
		cv::Size size( (2*img.rows/3)*templ_original.cols/templ_original.rows,2*img.rows/3); //resize templates. In this case, parameter: 2/3 is only suitable for those source images in database.
		std::cout << "size:" << size << std::endl;
		cv::resize(templ_original,templ,size);//resize image
		std::cout << "template size: " << templ.cols<<"x"<<templ.rows<< std::endl;
		letters=rawname;
		/// Create windows
		cv::namedWindow( image_window, CV_WINDOW_AUTOSIZE );
		cv::namedWindow( result_window, CV_WINDOW_AUTOSIZE );
		cv::namedWindow( "region of interest", CV_WINDOW_AUTOSIZE );
		  /// Source image to display
		cv::Mat img_display;
		img.copyTo( img_display );
  
		/// Create the result matrix
		int result_cols =  img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;
		result.create( result_cols, result_rows, CV_32FC1 );
  
		/// Do the Matching
		cv::Mat img_roi = img(cv::Rect(0,0,templ.cols,img.rows));//to speed up the program, only load a part of interest region
		matchTemplate( img_roi, templ, result, match_method );
		//matchTemplate( img, templ, result, match_method );
		//img_roi.copyTo( img_display );
		std::cout << "match_method:" << match_method <<std::endl;
		cv::imshow( "region of interest", img_roi );
		/// Localizing the best match with minMaxLoc
		double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
		cv::Point matchLoc;
		minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );

		//std::cout<<"maxVal: "<<maxVal<<endl;
		/// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
		if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED )
			{ matchLoc = minLoc; 
			score=minVal;}
		else  
			{ matchLoc = maxLoc; 
			score= maxVal;}

		std::cout << "current score: " << score<< std::endl;
		
		/// update the score when new score is higher
		if (score_pre > score)	
			{score = score_pre;
			letters = letters_pre;
			//matchLoc_pre = matchLoc;
			templ.copyTo( templ_pre );}
		else
			{
			//matchLoc = matchLoc_pre;
			templ_pre.copyTo( templ );}
		
		std::cout << "Best score so far: " << score<< std::endl;
		std::cout << "matchLoc: " << matchLoc<< std::endl;
		if (matchLoc.x<5)
		{score_pre = score;
		letters_pre=letters;}
		/// Show me what you got
		cv::rectangle( img_display, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
		cv::rectangle( result, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
		cv::imshow( image_window, img_display );
		cv::imshow( result_window, result );
		std::cout<<"current best match so far: "<<letters<<std::endl;
		//waitKey(0);
		std::cout<<"-----------------------------------"<<std::endl;
	}
  time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
  std::cout << "Detected letters/numbers are :"<<  letters << std::endl;
  std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
  cv::waitKey(0);
  return 0;
}
