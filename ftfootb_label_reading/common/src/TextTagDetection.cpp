#include "ftfootb_label_reading/TextTagDetection.h"

TextTagDetection::TextTagDetection(const std::string& path_data)
{
	// Load Text cascade (.xml file)
	std::string text_tags_cascade_xml_file = path_data + "TextLabelClassifier/haarclassifier_new/cascade.xml";
	text_tags_cascade_.load(text_tags_cascade_xml_file);

	// read text tag template image
	std::string tag_template_fullname = path_data + "template_with_small_rim.png";
	text_tag_template_image_ = cv::imread(tag_template_fullname, CV_LOAD_IMAGE_GRAYSCALE);
	text_tag_template_target_size_ = cv::Size(110,20);	//cv::Size(55,10);
	cv::resize(text_tag_template_image_, text_tag_template_image_, text_tag_template_target_size_);
}

// struct byArea and byCenterX are meant to compare the area
//and x coordinate of two rectangles.
struct byArea
{
    bool operator () (const cv::Rect & a,const cv::Rect & b)
    {
         return a.width*a.height > b.width*b.height ;
    }
};

struct byCenterX
{
    bool operator () (const cv::Rect & a,const cv::Rect & b)
    {
         return a.x+0.5*a.width < b.x+0.5*b.width;
    }
};

////////////////////////////////////////    Hough line transform  /////////////////////////////////////////////


//this function is to find the best line with most white white pixels.
//The input of this function is a map between the lines position (represented by x or y coordinate)
//and the white pixels number. The output of the function is the line position with largest white pixels number.
inline float TextTagDetection::find_best_two_lines(const std::map<int,float>& lines_with_count_map)
{
	if (lines_with_count_map.size() > 0)
		return (--lines_with_count_map.end())->second;

	return 0.f;
}


// This function is to count the number of the white pixels.
int TextTagDetection::count_white_pixels_on_line(const cv::Mat& dst, const double r, const double cosine, const double sine, const bool vertical)
{
	// line equation: y = (r-cosine*x)/sine
	int count = 0;
	if (vertical)
	{
		for(int y=0; y<dst.rows; ++y)
		{
			int x = (r-y*sine)/cosine;
			if (dst.at<uchar>(y,x)==255)
				count++;
		}
	}
	else
	{
		for(int x=0; x<dst.cols; ++x)
		{
			int y = (r-cosine*x)/sine;
			if (dst.at<uchar>(y,x)==255)
				count++;
		}
	}
	return count;
}

//#define _DEBUG_DISPLAYS_
cv::Rect TextTagDetection::get_rect_with_hough_line_transform(const cv::Mat& src)
{
	// preprocessing the input text tag
	double resize_fy = 100. / (double)src.rows;
	double resize_fx = resize_fy;	//400. / (double)src.cols;
	std::map<int,float> upper_horizontal_with_count_map, lower_horizontal_with_count_map,	left_vertical_with_count_map, right_vertical_with_count_map;
	cv::Mat src_resized, dst, cdst;
	cv::resize(src, src_resized, cv::Size(), resize_fx, resize_fy);
	src_resized.convertTo(src_resized, -1, 0.8, 0);		// todo: cv::normalize(src_resized, src_resized, 0, 255, cv::NORM_MINMAX);
	cv::Mat img_thres_gray;
	double canny_acc_thresh = cv::threshold(src_resized, img_thres_gray, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
//	double CannyThresh = 0.1 * canny_acc_thresh;
	cv::Canny(src_resized, dst, 0, canny_acc_thresh/2);

#ifdef _DEBUG_DISPLAYS_
	cvtColor(dst, cdst, CV_GRAY2BGR);
#endif

	// detect horizontal lines
	std::vector<cv::Vec2f> lines_horizontal;
	cv::HoughLines(dst, lines_horizontal, 1, CV_PI/180, 0.1*dst.cols, 0, 0 );
	for( size_t i = 0; i < lines_horizontal.size(); i++ )
	{
		double rho = lines_horizontal[i][0], theta = lines_horizontal[i][1];

		if(theta>CV_PI/180*88 && theta<CV_PI/180*92)
		{
			cv::Point pt1, pt2;
			double c = cos(theta), s = sin(theta);
			double x0 = c*rho, y0 = s*rho;
			pt1.x = 0;		//cvRound(x0 + 1000*(-s));
			pt1.y = rho/s;	//cvRound(y0 + 1000*(c));
			pt2.x = dst.cols;		//cvRound(x0 - 1000*(-s));
			pt2.y = (rho - dst.cols*c)/s;	//cvRound(y0 - 1000*(c));
			int count = count_white_pixels_on_line(dst, rho, c, s, false);
			if (pt1.y<0.25*dst.rows)
				upper_horizontal_with_count_map[count]=0.5*(pt1.y+pt2.y);
			else if (pt1.y>0.75*dst.rows)
				lower_horizontal_with_count_map[count]=0.5*(pt1.y+pt2.y);
#ifdef _DEBUG_DISPLAYS_
			cv::line(cdst, pt1, pt2, cv::Scalar(0,count/(double)dst.cols*255,0), 2, CV_AA);
#endif
		}
	}

	// detect vertical lines
	std::vector<cv::Vec2f> lines_vertical;
	cv::HoughLines(dst, lines_vertical, 1, CV_PI/180, 0.2*dst.rows, 0, 0 );
	for( size_t i = 0; i < lines_vertical.size(); i++ )
	{
		double rho = lines_vertical[i][0], theta = lines_vertical[i][1];
		if(theta>CV_PI/180*178 || theta<CV_PI/180*2)
		{
			cv::Point pt1, pt2;
			double c = cos(theta), s = sin(theta);
			double x0 = c*rho, y0 = s*rho;
			pt1.x = rho/c;		//cvRound(x0 + 1000*(-s));
			pt1.y = 0;			//cvRound(y0 + 1000*(c));
			pt2.x = (rho - dst.rows*s)/c;	//cvRound(x0 - 1000*(-s));
			pt2.y = dst.rows;				//cvRound(y0 - 1000*(c));
			int count = count_white_pixels_on_line(dst, rho, c, s, true);
			if (pt1.x>0.92*dst.cols)
				right_vertical_with_count_map[count]=0.5*(pt1.x+pt2.x);
			else if (pt1.x<0.08*dst.cols)
				left_vertical_with_count_map[count]=0.5*(pt1.x+pt2.x);
#ifdef _DEBUG_DISPLAYS_
			cv::line( cdst, pt1, pt2, cv::Scalar(0,count/(double)dst.rows*255,0), 2, CV_AA);
#endif
		}
	}

	//divide the text tag into four parts(left right upper and lower)
	//and obtain the position with largest white pixels number for every part
	// if the obtained position is too small then set best to be the boundries of the  input image
	float best_left_vertical = find_best_two_lines(left_vertical_with_count_map);
	float best_right_vertical = find_best_two_lines(right_vertical_with_count_map);
	float best_upper_horizontal = find_best_two_lines(upper_horizontal_with_count_map);
	float best_lower_horizontal = find_best_two_lines(lower_horizontal_with_count_map);

	if (best_lower_horizontal<=0.f)
		best_lower_horizontal=dst.rows;

	if (best_right_vertical<=0.f)
		best_right_vertical=dst.cols;

	//remember that we have scaled the image, so here is the operation to
	//restore the lines' position back to its original size.
	cv::Rect rectangle(best_left_vertical/resize_fx, best_upper_horizontal/resize_fy, best_right_vertical/resize_fx-best_left_vertical/resize_fx, best_lower_horizontal/resize_fy-best_upper_horizontal/resize_fy);

//	std::cout<<rectangle.x<<" "<<rectangle.y<<" "<<rectangle.width<<" "<<rectangle.height<<std::endl;

#ifdef _DEBUG_DISPLAYS_
	cv::imshow("lines", cdst);
	cv::imshow("dst", dst);
	cv::imshow("cut", src(rectangle));
	cv::waitKey();
#endif

	return rectangle;
}


////////////////////////////////////////    Dashes detection  /////////////////////////////////////////////

cv::Rect TextTagDetection::restore_text_tag_by_detected_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image)
{
	cv::Rect refined_text_tag = text_tag;
	if (detected_dashes_list.size()==3)
	{
		refined_text_tag = restore_text_tag_by_three_detected_dashes(detected_dashes_list, text_tag, image);
	}
	else if (detected_dashes_list.size()==2)
	{
		// center0 is left of center1
		cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
		cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;

		// 3 possibilities for the missing dash
		cv::Point center_l = cv::Point(center0.x-(center1.x-center0.x),center0.y);
		cv::Point center_m = cv::Point(center0.x+0.5*(center1.x-center0.x),center0.y);
		cv::Point center_r = cv::Point(center1.x+(center1.x-center0.x),center1.y);

		cv::Rect text_tag_l = restore_tag_by_estimated_dashes(center_l, text_tag, image, detected_dashes_list);
		cv::Rect text_tag_m = restore_tag_by_estimated_dashes(center_m, text_tag, image, detected_dashes_list);
		cv::Rect text_tag_r = restore_tag_by_estimated_dashes(center_r, text_tag, image, detected_dashes_list);

//		std::cout<<"text_tag l: "<<text_tag_l.x<<" "<<text_tag_l.y<<" "<<text_tag_l.width<<" "<<text_tag_l.height<<std::endl;
//		std::cout<<"text_tag r: "<<text_tag_r.x<<" "<<text_tag_r.y<<" "<<text_tag_r.width<<" "<<text_tag_r.height<<std::endl;
//		std::cout<<"text_tag m: "<<text_tag_m.x<<" "<<text_tag_m.y<<" "<<text_tag_m.width<<" "<<text_tag_m.height<<std::endl;
		refined_text_tag = select_best_match_from_three_estimated_dashes(text_tag_l, text_tag_m, text_tag_r, text_tag, image);
	}

	return refined_text_tag;
}

//cv::Rect restore_text_tag_by_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img)
//{
//	cv::Mat roi=img(text_tag);
//	if (detected_dashes_list.size()==3 )
////		&& (double(detected_dashes_list[0].y+0.5*detected_dashes_list[0].height)-
////											double(detected_dashes_list[1].y+0.5*detected_dashes_list[1].height))<roi.rows/25
////											&& std::fabs( double(detected_dashes_list[2].x)- double(detected_dashes_list[1].x)
////											- double(detected_dashes_list[1].x)-double(detected_dashes_list[0].x) )<roi.cols/25 )
//	{
//		cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
//		cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
//		cv::Point center2 = (detected_dashes_list[2].tl()+detected_dashes_list[2].br())*0.5;
//		std::cout<<"center0: "<<center0.x<<" "<<center0.y<<std::endl;
//		std::cout<<"center1: "<<center1.x<<" "<<center1.y<<std::endl;
//		std::cout<<"center2: "<<center2.x<<" "<<center2.y<<std::endl;
//		cv::Point tag_center;
//		double two_near_dashes_gap = std::fabs( 0.5*(double(detected_dashes_list[2].x) - double(detected_dashes_list[0].x)) );
//		tag_center.x=ceil((double(center1.x-text_tag.x)-2./140.*two_near_dashes_gap)+text_tag.x);
//		tag_center.y=ceil((97./53.*double(center1.y-text_tag.y)*0.5)+text_tag.y);
//		std::cout<<"tag_center.x "<<tag_center.x<<std::endl;
//		std::cout<<"tag_center.y "<<tag_center.y<<std::endl;
//		text_tag.width = ceil(53./14.*two_near_dashes_gap);//two_near_dashes_gap*2 + 127* two_near_dashes_gap/140+ 123* two_near_dashes_gap/140;
//		text_tag.height = ceil(97./140.*two_near_dashes_gap);
//		text_tag.x= tag_center.x-0.5*text_tag.width;
//		text_tag.y= tag_center.y-0.5*text_tag.height;
//
//		std::cout<<"text_tag: "<<text_tag.x<<" "<<text_tag.y<<" "<<text_tag.width<<" "<<text_tag.height<<std::endl;
//	}
//	return text_tag;
//}

// sharpen images
void TextTagDetection::unsharpMask(cv::Mat& im)
{
    cv::Mat tmp;
    cv::GaussianBlur(im, tmp, cv::Size(5,5), 0);
    cv::addWeighted(im, 1.5, tmp, -0.5, 0, im);
}

double TextTagDetection::angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

// this function is to detect the dashes by finding rectangle contours.
void TextTagDetection::detect_dashes(const cv::Rect& rect, const cv::Mat& image, std::vector<cv::Rect>& detected_dashes_list)
{
		cv::Mat src;
		double resize_fy = 150. / (double)rect.height;
		double resize_fx = resize_fy;	//600. / (double)rect.width;
		cv::resize(image(rect), src, cv::Size(), resize_fx, resize_fy);

		cv::Mat gray1,gray2,gray3,gray4;
		cv::Mat bw1,bw2,bw3,bw4;
		cv::Mat src1,src2,src3,src4;
		//check if the image is empty
		if (src.empty())
			std::cout<<"[detect dashed ERROR] could not load the image."<<std::endl;
		src.convertTo(src, -1, 1.5, 0);
	//	while(1)
	//		{

		//1. normal image 2. Gaussian blur 3. sharpen image 4. very bright image
		src.convertTo(src1, -1, 1.45, 0);
		cv::GaussianBlur(src, src2, cv::Size(3,3),0);
		src3=src.clone();
		unsharpMask(src3);
		src.convertTo(src4, -1, 2.85, 0);

		// Use Canny instead of threshold to catch squares with gradient shading on 3 conditions
		double CannyAccThresh = cv::threshold(src1,gray1,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
		cv::Canny(src1,bw1,0,CannyAccThresh*1);

		CannyAccThresh = cv::threshold(src2,gray2,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
		cv::Canny(src2,bw2,0,CannyAccThresh*1);

		CannyAccThresh = cv::threshold(src3,gray3,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
		cv::Canny(src3,bw3,0,CannyAccThresh*1);

		CannyAccThresh = cv::threshold(src4,gray4,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
		cv::Canny(src4,bw4,0,CannyAccThresh*1);

		// Find contours on 3 conditions and combine them
		std::vector<std::vector<cv::Point> > contours,contours1,contours2,contours3,contours4;
		cv::findContours(bw1, contours1, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
		cv::findContours(bw2, contours2, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
		cv::findContours(bw3, contours3, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
		cv::findContours(bw4, contours4, CV_RETR_EXTERNAL , CV_CHAIN_APPROX_SIMPLE);
		//Most of the intersections can be filtered with CV_RETR_EXTERNAL flag.
		contours.reserve( contours1.size() + contours2.size() + contours3.size()+ contours4.size()); // preallocate memory for contours vector
		contours.insert( contours.end(), contours1.begin(), contours1.end() );
		contours.insert( contours.end(), contours2.begin(), contours2.end() );
		contours.insert( contours.end(), contours3.begin(), contours3.end() );
		contours.insert( contours.end(), contours4.begin(), contours4.end() );

	//		std::cout<<contours.size()<<" "<<contours1.size()<<" "<<contours2.size()<<" "<<contours3.size()<<std::endl;
		for (unsigned int i = 0; i < contours.size(); i++)
		{
			// Approximate contour with accuracy proportional
			// to the contour perimeter
			std::vector<cv::Point> approx;
			cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.03, true);

			// Skip small or non-convex objects
			if (std::fabs(cv::contourArea(contours[i])) < 3)
				continue;
			else if (approx.size() >= 4 && approx.size() <= 6)
			{
				// Number of vertices of polygonal curve
				int vtc = approx.size();

				// Get the cosines of all corners
				std::vector<double> cos;
				for (int j = 2; j < vtc+1; j++)
					cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

				// Sort ascending the cosine values
				std::sort(cos.begin(), cos.end());

				// Get the lowest and the highest cosine
				double mincos = cos.front();
				double maxcos = cos.back();

				// Use the degrees obtained above and the number of vertices
				// to determine the shape of the contour
				if (vtc == 4 && mincos >= -0.23 && maxcos <= 0.3)
				{
					cv::Rect bbrect = cv::boundingRect(contours[i]);
					cv::Rect rect_original_size(bbrect.x/resize_fx, bbrect.y/resize_fy, bbrect.width/resize_fx, bbrect.height/resize_fy);
					detected_dashes_list.push_back(rect_original_size);
				}
			}
		}

		// transfer the dashes coordinates from roi to img and also show them
		for (unsigned int p = 0; p < detected_dashes_list.size(); ++p)
		{
			detected_dashes_list[p].x += rect.x;
			detected_dashes_list[p].y += rect.y;
		}

//		std::cout << "detected_dashes_list.size()=" << detected_dashes_list.size() << std::endl;
//		displayDashes(detected_dashes_list, cv::Scalar(0,255,0), image);
		find_right_dashes(detected_dashes_list, rect);
//		std::cout << "detected_dashes_list.size()=" << detected_dashes_list.size() << std::endl;
//		displayDashes(detected_dashes_list, cv::Scalar(0,0,255), image);
}

void TextTagDetection::displayDashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Scalar& color, const cv::Mat& image)
{
	cv::Mat image_display;
	cv::cvtColor(image, image_display, CV_GRAY2BGR);
	for (unsigned int p = 0; p < detected_dashes_list.size(); ++p)
	{
		cv::rectangle(image_display, detected_dashes_list[p], color, 1);
	}
	cv::imshow("dashes", image_display);
	cv::waitKey();
}

// For the case of two dashes are detected, three text tag will be estimated.
cv::Rect TextTagDetection::restore_tag_by_estimated_dashes(const cv::Point& estimated_dash_center, const cv::Rect& text_tag, const cv::Mat& image, std::vector<cv::Rect> detected_dashes_list)
{
	cv::Rect rect_temp(estimated_dash_center.x-0.5*detected_dashes_list[0].width, estimated_dash_center.y-0.5*detected_dashes_list[0].height, detected_dashes_list[0].width, detected_dashes_list[0].height);
	detected_dashes_list.push_back(rect_temp);

	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byCenterX());
//	std::cout<<"detected_dashes_list: "<<detected_dashes_list[0].x<<detected_dashes_list[1].x<<detected_dashes_list[2].x<<std::endl;
	cv::Rect text_tag_candidate = restore_text_tag_by_three_detected_dashes(detected_dashes_list, text_tag, image);

	return text_tag_candidate;
}

//double TextTagDetection::compare_detection_with_template(cv::Rect text_tag, cv::Mat img,std::string package_path)
//{
//	std::string tag_template_fullname=package_path.append("template.png");
//	std::cout<<"tag_template_fullname: "<<tag_template_fullname<<std::endl;
//	int match_method=CV_TM_CCOEFF_NORMED;//CV_TM_CCOEFF_NORMED
//	cv::Mat template_image = cv::imread(tag_template_fullname,CV_LOAD_IMAGE_GRAYSCALE);
//	cv::Mat source_image=img(text_tag);
//	cv::adaptiveThreshold(template_image, template_image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, -5);
//	cv::adaptiveThreshold(source_image, source_image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, -5);
//
//	cv::Size dsize(55,10);
//	cv::resize(template_image,template_image,dsize);
//	cv::resize(source_image,source_image,dsize);
//	double minVal, maxVal,score=0.;
//	cv::Point minLoc, maxLoc,matchLoc;
//	cv::Mat result;
//	matchTemplate(source_image,template_image,result,match_method);
//
//	cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
//	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
//	{
//		matchLoc = minLoc;
//		score=minVal;
//	}
//	else
//	{
//		matchLoc = maxLoc;
//		score= 1.-maxVal;
//	}
//	return score;
//}

// this function is to find the best estimated text tag generated by two dashes with computing their similarity with a template.
cv::Rect TextTagDetection::select_best_match_from_three_estimated_dashes(const cv::Rect& text_tag_l, const cv::Rect& text_tag_m, const cv::Rect& text_tag_r,
																					const cv::Rect& text_tag, const cv::Mat& image)
{
	bool left=true, middle=true, right=true;
	if (text_tag_l.area()<0.25*text_tag.area())
		left=false;
	if (text_tag_m.area()<0.25*text_tag.area() || text_tag_m.width<0.6*text_tag.width)
		middle=false;
	if (text_tag_r.area()<0.25*text_tag.area())
		right=false;

	double score_l = 1e10, score_m = 1e10, score_r = 1e10;
	if (left == true)
		score_l = compare_detection_with_template(image(text_tag_l));
	if (middle == true)
		score_m = compare_detection_with_template(image(text_tag_m));
	if (right == true)
		score_r = compare_detection_with_template(image(text_tag_r));

	std::map<double, cv::Rect> ranking;
	ranking[score_l] = text_tag_l;
	ranking[score_m] = text_tag_m;
	ranking[score_r] = text_tag_r;
	cv::Rect best_text_tag = text_tag;
	if (score_l<1e10 || score_m<1e10 || score_r<1e10)
		best_text_tag = ranking.begin()->second;

	return best_text_tag;
}

double TextTagDetection::compare_detection_with_template(const cv::Mat& image)
{
	const int match_method=CV_TM_SQDIFF_NORMED;//CV_TM_CCOEFF_NORMED

	cv::Mat image_resized;
	cv::resize(image, image_resized, text_tag_template_target_size_);
	cv::threshold(image_resized, image_resized, -1, 255, cv::THRESH_BINARY|cv::THRESH_OTSU);
	double minVal, maxVal, score = 0;
	cv::Point minLoc, maxLoc, matchLoc;
	cv::Mat result;
	cv::matchTemplate(image_resized, text_tag_template_image_, result, match_method);
	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score = 1.-minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score = maxVal;
	}

//	std::cout<<"score: "<<score<<std::endl;
//	cv::imshow("theshold", image_resized);
//	cv::imshow("template", text_tag_template_image_);
//	cv::waitKey();

	return score;
}

//Eliminate some dashes by their shapes or aspect ratio etc and merge intersecting dashes.
void TextTagDetection::find_right_dashes(std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& rect)
{
	std::vector<cv::Rect> non_intersecting_detected_dashes_list;
	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byArea());
	for(unsigned int i=0; i < detected_dashes_list.size(); i++)
	{
	    bool toAdd = true;
	    cv::Point center = (detected_dashes_list[i].tl()+detected_dashes_list[i].br())*0.5;
	    if (detected_dashes_list[i].height>=detected_dashes_list[i].width)
			toAdd = false;

	    if (double(detected_dashes_list[i].width*detected_dashes_list[i].height)>=0.02*double(rect.width*rect.height))
			toAdd = false;

	    if ((center.y-rect.y)>0.85*rect.height||(center.y-rect.y)<0.15*rect.height)
	   		toAdd = false;

	    if (double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)<1.5 || double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)>4)
	    	toAdd = false;

	    for(unsigned int j=0; j < non_intersecting_detected_dashes_list.size(); j++)
	    {
	        if (non_intersecting_detected_dashes_list[j].contains(center))
	        {
	            toAdd = false;
	            break;
	        }
	    }

	    if (toAdd)
	    	non_intersecting_detected_dashes_list.push_back(detected_dashes_list[i]);
	 }
	std::sort(non_intersecting_detected_dashes_list.begin(), non_intersecting_detected_dashes_list.end(), byCenterX());

	detected_dashes_list = non_intersecting_detected_dashes_list;
}

//restore text tag by THREE dashes.
cv::Rect TextTagDetection::restore_text_tag_by_three_detected_dashes(const std::vector<cv::Rect>& detected_dashes_list, const cv::Rect& text_tag, const cv::Mat& image)
{
	cv::Rect refined_text_tag;
//	cv::Rect text_tag_original=text_tag;
//	cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
	cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
//	cv::Point center2 = (detected_dashes_list[2].tl()+detected_dashes_list[2].br())*0.5;

	cv::Point tag_center;
	double two_near_dashes_gap_x = std::fabs( 0.5*(double(detected_dashes_list[2].x) - double(detected_dashes_list[0].x)) );
//	tag_center.x=ceil((double(center1.x-text_tag.x)-2./140.*two_near_dashes_gap_x)+text_tag.x);
//	tag_center.y=ceil((97./53.*double(center1.y-text_tag.y)*0.5)+text_tag.y);

	// perfect tag template: width=690mm, height=129mm, width from dash to next dash=179.5mm, vertical distance between tag upper border and dash center=71
	refined_text_tag.width = ceil(690./179.5*two_near_dashes_gap_x);		//two_near_dashes_gap*2 + 127* two_near_dashes_gap/140+ 123* two_near_dashes_gap/140;
	refined_text_tag.height = ceil(129./690.*refined_text_tag.width);		//ceil(129./640.*refined_text_tag.width); might work better
	refined_text_tag.x = center1.x-0.5*refined_text_tag.width;
	refined_text_tag.y = center1.y-6.5/129.*refined_text_tag.height-0.5*refined_text_tag.height;		// -6.5/129.*refined_text_tag.height is a compensation for the dash not being perfectly centered vertically
//	std::cout<<"text_tag: "<<text_tag.x<<" "<<text_tag.y<<" "<<text_tag.width<<" "<<text_tag.height<<std::endl;
	if(refined_text_tag.x<0 || refined_text_tag.y<0 || (refined_text_tag.x+refined_text_tag.width)>=image.cols || (refined_text_tag.y+refined_text_tag.height)>=image.rows)
		refined_text_tag = cv::Rect(0, 0, 1, 1);
	return refined_text_tag;
}


////////////////////////////////////////    Viola-Jones Detector  /////////////////////////////////////////////

inline void TextTagDetection::text_tag_detection_with_VJ(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list)
{
//	cv::cvtColor(image, image, CV_RGB2GRAY);
//	cv::equalizeHist(image, image);

	text_tags_cascade_.detectMultiScale(image, rectangle_list, 1.1, 3, 0, cv::Size(35, 9), cv::Size());		// (image, rectangle_list, 1.1, 10, 0, cv::Size(35, 9), cv::Size())
//    if (rectangle_list.size() == 0)
//    {
//
//    	text_tags_cascade.detectMultiScale(image, rectangle_list, 1.07, 1 , 0 ,cv::Size());
//    }
//
//    if (rectangle_list.size() == 0)
//
//    {
//    	text_tags_cascade.detectMultiScale(image, rectangle_list, 1.03, 1 , 0,cv::Size());
//    }
//
}

/////////////////////  text tag Detector integrated by VJ Hough transform and dashes detection /////////////////////////

void TextTagDetection::text_tag_detection_fine_detection_vj(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list)
{
	std::vector<cv::Rect> initial_rectangle_list;
	text_tag_detection_with_VJ(image, initial_rectangle_list);

	for (std::vector<cv::Rect>::const_iterator r = initial_rectangle_list.begin(); r != initial_rectangle_list.end(); r++)
	{
		// update detection by Hough line transform
		cv::Mat roi = image(*r);
		cv::Rect rectangle_hough_roi = get_rect_with_hough_line_transform(roi);
		cv::Rect rectangle_updated_by_hough_line(r->x+rectangle_hough_roi.x, r->y+rectangle_hough_roi.y, rectangle_hough_roi.width, rectangle_hough_roi.height);

		//update detection by dashes detection
		std::vector<cv::Rect> detected_dashes_list;
		detect_dashes(rectangle_updated_by_hough_line, image, detected_dashes_list);
		cv::Rect rectangle_updated_by_dashes_detection = restore_text_tag_by_detected_dashes(detected_dashes_list, rectangle_updated_by_hough_line, image);

//		double score = compare_detection_with_template(rectangle_updated_by_dashes_detection,image,package_path);
//
//		std::cout<<"score: "<<score<<std::endl;

//		cv::imshow("Viola-Jones", image(*r));
//		cv::imshow("Hough", image(rectangle_updated_by_hough_line));
//		cv::imshow("Dashes", image(rectangle_updated_by_dashes_detection));
//		cv::waitKey();

		rectangle_list.push_back(rectangle_updated_by_dashes_detection);
	}
}

void TextTagDetection::text_tag_detection_fine_detection_rectangle_detection(const cv::Mat& image, std::vector<cv::Rect>& rectangle_list, std::vector<TagDetectionData>& detections_r)
{
	// detect rectangles
	std::vector<cv::Rect> initial_rectangle_list;
	std::vector<TagDetectionData> initial_detections_r;
	detect_tag_by_frame(image, initial_rectangle_list, initial_detections_r);

	for (std::vector<cv::Rect>::const_iterator r = initial_rectangle_list.begin(); r != initial_rectangle_list.end(); r++)
	{
//		//update detection by dashes detection
//		std::vector<cv::Rect> detected_dashes_list;
//		detect_dashes(*r, image, detected_dashes_list);
//		cv::Rect rectangle_updated_by_dashes_detection = restore_text_tag_by_detected_dashes(detected_dashes_list, *r, image);
//		rectangle_list.push_back(rectangle_updated_by_dashes_detection);

		// verify with template
		double score = compare_detection_with_template(image(*r));
		if (score > 0.3)
			rectangle_list.push_back(*r);
	}

	for (std::vector<TagDetectionData>::const_iterator r = initial_detections_r.begin(); r != initial_detections_r.end(); r++)
	{
		// get rectified image region
		cv::Mat rectified_image;
		remove_projection(*r, image, rectified_image);

		// verify with template
		double score = compare_detection_with_template(rectified_image);
		if (score > 0.3)
			detections_r.push_back(*r);
	}
}


void TextTagDetection::cut_out_rotated_rectangle(const cv::RotatedRect& rotated_rect, const cv::Mat& image, cv::Mat& rotated_image)
{
	double diagonal = sqrt(rotated_rect.size.width*rotated_rect.size.width + rotated_rect.size.height*rotated_rect.size.height);
	cv::Mat roi(diagonal, diagonal, CV_8UC1);
	int src_x = rotated_rect.center.x-diagonal*0.5;
	int src_y = rotated_rect.center.y-diagonal*0.5;
	for (int v=0; v<roi.rows; ++v)
		for (int u=0; u<roi.cols; ++u)
		{
			if (src_x+u<0 || src_y+v<0)
				roi.at<uchar>(v,u) = 0;
			else
				roi.at<uchar>(v,u) = image.at<uchar>(src_y+v, src_x+u);
		}
	cv::Mat rotation_matrix = cv::getRotationMatrix2D(cv::Point2f(diagonal*0.5, diagonal*0.5), rotated_rect.angle, 1.0);
    cv::warpAffine(roi, rotated_image, rotation_matrix, cv::Size(diagonal, diagonal));
	rotated_image = rotated_image(cv::Rect((diagonal-rotated_rect.size.width)*0.5, (diagonal-rotated_rect.size.height)*0.5, rotated_rect.size.width, rotated_rect.size.height));

//    cv::imshow("rotated_image", rotated_image);
//    cv::waitKey();
}


void TextTagDetection::remove_projection(const cv::RotatedRect& rotated_rect, const cv::Mat& image, cv::Mat& rectified_image)
{
	// setup corresponding points
	std::vector<cv::Point2f> image_coordinates(4), target_coordinates(4);
	cv::Point2f rect_points[4];
	rotated_rect.points(rect_points);
	for (int i=0; i<4; ++i)
		image_coordinates[i] = rect_points[i];
	target_coordinates[0] = cv::Point2f(0, rotated_rect.size.height-1);
	target_coordinates[1] = cv::Point2f(0, 0);
	target_coordinates[2] = cv::Point2f(rotated_rect.size.width-1, 0);
	target_coordinates[3] = cv::Point2f(rotated_rect.size.width-1, rotated_rect.size.height-1);

	// find homography and rectify image
	cv::Mat H = cv::findHomography(image_coordinates, target_coordinates);
	cv::warpPerspective(image, rectified_image, H, rotated_rect.size);

//    cv::imshow("rectified_image", rectified_image);
//    cv::waitKey();
}


void TextTagDetection::remove_projection(const TagDetectionData& detection, const cv::Mat& image, cv::Mat& rectified_image)
{
	// setup corresponding points
	std::vector<cv::Point2f> target_coordinates(4);
	target_coordinates[0] = cv::Point2f(0, detection.min_area_rect_.size.height-1);
	target_coordinates[1] = cv::Point2f(0, 0);
	target_coordinates[2] = cv::Point2f(detection.min_area_rect_.size.width-1, 0);
	target_coordinates[3] = cv::Point2f(detection.min_area_rect_.size.width-1, detection.min_area_rect_.size.height-1);

	// find homography and rectify image
	cv::Mat H = cv::findHomography(detection.corners_, target_coordinates);
	cv::warpPerspective(image, rectified_image, H, detection.min_area_rect_.size);

//    cv::imshow("rectified_image", rectified_image);
//    cv::waitKey();
}


void TextTagDetection::detect_tag_by_frame(const cv::Mat& image_grayscale, std::vector<cv::Rect>& detections, std::vector<TagDetectionData>& detections_r)
{
	detections.clear();
	detections_r.clear();

	// Use Canny instead of threshold to catch squares with gradient shading on 3 conditions
	//double CannyAccThresh = cv::threshold(image_grayscale, image_bw, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Mat edge_image, binary_image;
	double canny_threshold = cv::threshold(image_grayscale, binary_image, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
	cv::Canny(image_grayscale, edge_image, 0.5*canny_threshold, canny_threshold);

	// Find contours on 3 conditions and combine them
	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(edge_image.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

//	cv::Mat display_image;
//	cv::cvtColor(image_grayscale, display_image, CV_GRAY2BGR);
//	cv::drawContours(display_image, contours, -1, CV_RGB(0,255,0), 1);
//	cv::imshow("contours", display_image);
//	cv::imshow("edge_image", edge_image);
//	cv::waitKey();

	for (size_t i=0; i<contours.size(); ++i)
	{
		// Skip small or non-convex objects
		if (std::fabs(cv::contourArea(contours[i])) < 1000)
			continue;

		// Approximate contour with accuracy proportional
		// to the contour perimeter
		std::vector<cv::Point> approx;
		cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.03, true);

		// Skip small or non-convex objects
		if (approx.size()!=4 || cv::isContourConvex(cv::Mat(approx))==false)
			continue;

		// verify that angles are about 90deg
		// Number of vertices of polygonal curve
		int vtc = approx.size();

		// Get the cosines of all corners
		std::vector<double> cos;
		for (int j = 2; j < vtc+1; j++)
			cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));

		// Sort ascending the cosine values
		std::sort(cos.begin(), cos.end());

		// Get the lowest and the highest cosine
		double mincos = cos.front();
		double maxcos = cos.back();

		// Use the degrees obtained above and the number of vertices
		// to determine the shape of the contour
		if ((mincos >= -0.3 && maxcos <= 0.3) == false)
			continue;

		// check for aspect ratio
		cv::RotatedRect tag_frame_r = cv::minAreaRect(contours[i]);
		while (tag_frame_r.angle < -45.)
		{
			tag_frame_r.angle += 90.;
			float width = tag_frame_r.size.height;
			tag_frame_r.size.height = tag_frame_r.size.width;
			tag_frame_r.size.width = width;
		}
		while (tag_frame_r.angle > 45.)
		{
			tag_frame_r.angle -= 90.;
			float width = tag_frame_r.size.height;
			tag_frame_r.size.height = tag_frame_r.size.width;
			tag_frame_r.size.width = width;
		}
		if (tag_frame_r.size.height/(double)tag_frame_r.size.width < 0.85*129./690. || tag_frame_r.size.height/(double)tag_frame_r.size.width > 1.15*129./690.)
			continue;
		detections_r.push_back(TagDetectionData(tag_frame_r, approx));

		cv::Rect tag_frame = cv::boundingRect(contours[i]);
		if (tag_frame.height/(double)tag_frame.width < 0.85*129./690. || tag_frame.height/(double)tag_frame.width > 1.15*129./690.)
			continue;

		detections.push_back(tag_frame);
	}

//	// display
//	cv::Mat display_image;
//	cv::cvtColor(image_grayscale, display_image, CV_GRAY2BGR);
//	for (size_t i=0; i<detections.size(); ++i)
//		cv::rectangle(display_image, detections[i], CV_RGB(0,255,0), 2);
//	cv::imshow("detections", display_image);
//	cv::waitKey();
}




