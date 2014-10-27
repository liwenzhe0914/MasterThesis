#include "ftfootb_label_reading/MatchTemplate.h"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}

	std::string image_filename = argv[1];
	std::string path = argv[2];

	MatchTemplate mt(path);
	cv::Mat img = cv::imread(image_filename, 1);
	//std::cout << "source imagename: " << image_filename << std::endl;
	std::cout << "image size: " << img.cols<<"x"<<img.rows<< std::endl;

	double start_time;
	double time_in_seconds;
	start_time = clock();

	std::string tag_label;
	mt.read_tag(img, tag_label);
	std::cout << "The text tag reads: " << tag_label << "." << std::endl;

	time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;

	/*
//	double score_pre = -1e10;
	//for (unsigned int i = 0; i < templateImageNames.size(); i++)
	cv::Mat templ_pre;
	std::string letters_pre = "";
	double score=0.;
	double score_previous = -1e10;
	for (MatchTemplate::TokenTemplatesIterator it=letter_pair_templates.begin(); it!=letter_pair_templates.end(); ++it)
	{
		//std::cout << "current template imagename: " << allImageNames[i]<< std::endl;
		/// get image name without extension and path
//		int lastindex = templateImageNames[i].find_last_of(".");
//		int lastindex2 = templateImageNames[i].find_last_of("/");
//		std::string rawname = templateImageNames[i].substr(lastindex2+1, lastindex-lastindex2-1);
		std::cout << "Current template:" << it->first << std::endl;
		cv::Mat templ_original = it->second; // originalImage like it used to be, never modify
		cv::Size size( (2*img.rows/3)*templ_original.cols/templ_original.rows,2*img.rows/3); //resize templates. In this case, parameter: 2/3 is only suitable for those source images in database.
		std::cout << "size:" << size << std::endl;
		cv::Mat templ;
		cv::resize(templ_original,templ,size);//resize image
		std::cout << "template size: " << templ.cols<<"x"<<templ.rows<< std::endl;
		letters=it->first;
		/// Source image to display
		cv::Mat img_display;
		img.copyTo( img_display );

		/// Create the result matrix
		int result_cols =  img.cols - templ.cols + 1;
		int result_rows = img.rows - templ.rows + 1;
		cv::Mat result(result_cols, result_rows, CV_32FC1);

		/// Do the Matching
		int match_method = 5;
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
		if (score_previous > score)
			{score = score_previous;
			letters = letters_pre;
			//matchLoc_pre = matchLoc;
			templ.copyTo(templ_pre);}
		else
			{
			//matchLoc = matchLoc_pre;
			templ_pre.copyTo( templ );}

		std::cout << "Best score so far: " << score<< std::endl;
		std::cout << "matchLoc: " << matchLoc<< std::endl;
		if (matchLoc.x<5)
		{score_previous = score;
		letters_pre=letters;}
		/// Show me what you got
		cv::rectangle( img_display, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
		cv::rectangle( result, matchLoc, cv::Point( matchLoc.x + templ.cols , matchLoc.y + templ.rows ), cv::Scalar::all(0), 2, 8, 0 );
		cv::imshow( "Source Image", img_display );
		cv::imshow( "Result window", result );
		std::cout<<"current best match so far: "<<letters<<std::endl;
		//waitKey(0);
		std::cout<<"-----------------------------------"<<std::endl;
	}
	*/

	return 0;
}
