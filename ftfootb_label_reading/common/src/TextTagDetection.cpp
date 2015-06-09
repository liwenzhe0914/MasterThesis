#include "ftfootb_label_reading/TextTagDetection.h"

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
         return a.x+0.5*a.width> b.x+0.5*b.width ;
    }
};

////////////////////////////////////////    Hough line transform  /////////////////////////////////////////////


//this function is to find the best line with most white white pixels.
//The input of this function is a map between the lines position (represented by x or y coordinate)
//and the white pixels number. The output of the function is the line position with largest white pixels number.
float TextTagDetection::find_best_two_lines(std::map<float,float> lines_with_count_map)
{
	unsigned currentMax = 0;
	unsigned arg_max = 0;
	std::map<float,float>::iterator it = lines_with_count_map.begin();
	for(it=lines_with_count_map.begin(); it!=lines_with_count_map.end(); ++it )
	    {
		if (it->first > currentMax)
	    	{
	        	arg_max = it->second;
	        	currentMax = it->first;
	    	}
	    }
	return arg_max;
}


// This function is to count the number of the white pixels.
int TextTagDetection::count_white_pixels(cv::Mat dst,cv::Point pt1,cv::Point pt2,bool vertical)
{
	int count=0;
	cv::Point temp;
	if (vertical)
	{
		for(temp.y=-200;temp.y<200;temp.y++)
			{
				if (dst.at<uchar>(temp.y,pt2.x)==(255))
				{
					count++;
				}
			}
	}
	else
	{
		for(temp.x=pt1.x;temp.x<pt2.x;temp.x++)
		{
			if (dst.at<uchar>(pt1.y, temp.x)==(255))
			{
				count++;
			}
		}
	}
return count;
}

cv::Rect TextTagDetection::get_rect_with_hough_line_transform(cv::Mat src)
{
	// preprocessing the input text tag
//	cv::cvtColor(src, src, CV_RGB2GRAY);
	double resize_fx = 400. / src.cols;
	double resize_fy = 100. / src.rows;
	cv::Rect rectangle;
	std::map<float,float> upper_horizontal_with_count_map,lower_horizontal_with_count_map,
	left_vertical_with_count_map,right_vertical_with_count_map;
	cv::Mat dst,cdst;
	resize (src,src,cv::Size(),resize_fx,resize_fy);
	src.convertTo(src, -1, 0.8, 0);
	cv::Mat Img_Thres_Gray;
	double CannyAccThresh = threshold(src,Img_Thres_Gray,0,255,CV_THRESH_BINARY|CV_THRESH_OTSU);
//	double CannyThresh = 0.1 * CannyAccThresh;
	Canny(src,dst,0,CannyAccThresh/2);
	cvtColor(dst, cdst, CV_GRAY2BGR);

	// detect horizontal lines
	std::vector<cv::Vec2f> lines_horizontal;
	HoughLines(dst, lines_horizontal, 1, CV_PI/180, 40, 0, 0 );
	for( size_t i = 0; i < lines_horizontal.size(); i++ )
		{
		 float rho = lines_horizontal[i][0], theta = lines_horizontal[i][1];

		if(theta>CV_PI/180*89.5 && theta<CV_PI/180*90.5)
		{
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			cv::line( cdst, pt1, pt2, cv::Scalar(255,0,255), 3, CV_AA);
			int count=count_white_pixels(dst,pt1,pt2,0);
			if (pt1.y<0.25*cdst.rows)
			{
			upper_horizontal_with_count_map[count]=pt1.y;
			}
			else if (pt1.y>0.75*cdst.rows)
			{
			lower_horizontal_with_count_map[count]=pt1.y;
			}
		}
		}

	// detect vertical lines
	std::vector<cv::Vec2f> lines_vertical;
	HoughLines(dst, lines_vertical, 1, CV_PI/180, 20, 0, 0 );
	for( size_t i = 0; i < lines_vertical.size(); i++ )
	{
		float rho = lines_vertical[i][0], theta = lines_vertical[i][1];
		if(theta>CV_PI/180*179.5 || theta<CV_PI/180*0.5)
		{
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			cv::line( cdst, pt1, pt2, cv::Scalar(255,0,255), 3, CV_AA);
			int count=count_white_pixels(dst,pt1,pt2,1);

			if (pt1.x>0.92*cdst.cols)
			{
				right_vertical_with_count_map[count]=pt1.x;
			}
			else if (pt1.x<0.08*cdst.cols)
			{
				left_vertical_with_count_map[count]=pt1.x;
			}
		}
	}

	//divide the text tag into four parts(left right upper and lower)
	//and obtain the position with largest white pixels number for every part
	// if the obtained position is too small then set best to be the boundries of the  input image
	float best_left_vertical = find_best_two_lines(left_vertical_with_count_map);
	float best_right_vertical = find_best_two_lines(right_vertical_with_count_map);
	float best_upper_horizontal = find_best_two_lines(upper_horizontal_with_count_map);
	float best_lower_horizontal = find_best_two_lines(lower_horizontal_with_count_map);

	if (best_lower_horizontal==0)
	{
		best_lower_horizontal=100;
	}

	if (best_right_vertical==0)
	{
		best_right_vertical=400;
	}

	//remember that we have scaled the image, so here is the operation to
	//restore the lines' position back to its original size.

	rectangle.x=best_left_vertical/resize_fx;
	rectangle.y=best_upper_horizontal/resize_fy;
	rectangle.width=best_right_vertical/resize_fx-best_left_vertical/resize_fx;
	rectangle.height=best_lower_horizontal/resize_fy-best_upper_horizontal/resize_fy;

//	std::cout<<rectangle.x<<" "<<rectangle.y<<" "<<rectangle.width<<" "<<rectangle.height<<std::endl;

return rectangle;
}


////////////////////////////////////////    Dashes detection  /////////////////////////////////////////////

cv::Rect TextTagDetection::restore_text_tag_by_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img,std::string package_path)
{
	cv::Mat roi=img(text_tag);
	if (detected_dashes_list.size()==3 )
	{
		text_tag=restore_text_tag_by_three_detected_dashes(detected_dashes_list,text_tag,img);
	}
	else if (detected_dashes_list.size()==2)
	{
		cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
		cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
		std::vector<cv::Rect> detected_dashes_list_l=detected_dashes_list;
		std::vector<cv::Rect> detected_dashes_list_m=detected_dashes_list;
		std::vector<cv::Rect> detected_dashes_list_r=detected_dashes_list;

		cv::Point center_l=cv::Point (center1.x-(center0.x-center1.x),center0.y);
		cv::Point center_r=cv::Point (center0.x+(center0.x-center1.x),center0.y);
		cv::Point center_m=cv::Point (center0.x-0.5*(center0.x-center1.x),center0.y);


		cv::Rect text_tag_l,text_tag_m,text_tag_r;

		text_tag_l=restore_tag_by_estimated_dashes(center_l,text_tag,img,detected_dashes_list);
		text_tag_m=restore_tag_by_estimated_dashes(center_m,text_tag,img,detected_dashes_list);
		text_tag_r=restore_tag_by_estimated_dashes(center_r,text_tag,img,detected_dashes_list);

//		std::cout<<"text_tag l: "<<text_tag_l.x<<" "<<text_tag_l.y<<" "<<text_tag_l.width<<" "<<text_tag_l.height<<std::endl;
//		std::cout<<"text_tag r: "<<text_tag_r.x<<" "<<text_tag_r.y<<" "<<text_tag_r.width<<" "<<text_tag_r.height<<std::endl;
//		std::cout<<"text_tag m: "<<text_tag_m.x<<" "<<text_tag_m.y<<" "<<text_tag_m.width<<" "<<text_tag_m.height<<std::endl;
		text_tag=select_best_match_from_three_estimated_dashes(text_tag_l,text_tag_m,text_tag_r,text_tag,img,package_path);
	}

	return text_tag;
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
std::vector<cv::Rect> TextTagDetection::detect_dashes (cv::Rect rect,cv::Mat img)
{
		cv::Mat src =img(rect);
		cv::Mat src0=src.clone();
		double resize_fx = 600. / src.cols;
		double resize_fy = 150. / src.rows;
		cv::resize(src,src,cv::Size(),resize_fx,resize_fy);

		cv::Mat gray1,gray2,gray3,gray4;
		cv::Mat bw1,bw2,bw3,bw4;
		cv::Mat src1,src2,src3,src4;
		//check if the image is empty
		if (src.empty())
			std::cout<<"[detect dashed ERROR] could not load the image."<<std::endl;
		src.convertTo(src, -1, 1.5, 0);
	//	while(1)
	//		{
		std::vector<cv::Rect> detected_dashes_list;

		//1. normal image 2. Gaussian blur 3. sharpen image 4. very bright image
		src.convertTo(src1, -1, 1.45, 0);
		cv::GaussianBlur(src, src2, cv::Size(3,3),0);
		src3=src.clone();
		unsharpMask(src3);
		src.convertTo(src4, -1, 2.85, 0);
//		cv::cvtColor(src3, src3, CV_BGR2GRAY);
//		cv::cvtColor(src2, src2, CV_BGR2GRAY);
//		cv::cvtColor(src1, src1, CV_BGR2GRAY);
//		cv::cvtColor(src4, src4, CV_BGR2GRAY);

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
		std::vector<cv::Point> approx;


		for (unsigned int i = 0; i < contours.size(); i++)
		{
			// Approximate contour with accuracy proportional
			// to the contour perimeter
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
					cv::Rect rect = cv::boundingRect(contours[i]);
					cv::Rect rect_original_size;
					rect_original_size.x=rect.x/resize_fx;
					rect_original_size.y=rect.y/resize_fy;
					rect_original_size.width=rect.width/resize_fx;
					rect_original_size.height=rect.height/resize_fy;
					detected_dashes_list.push_back(rect_original_size);
				}
			}
		}

		//			transfer the dashes coordinates from roi to img and also show them
		std::vector<cv::Rect> detected_dashes_list_temp;
		for (unsigned int p = 0; p < detected_dashes_list.size(); p++)
			{

				cv::Rect r;
				r.x =detected_dashes_list[p].x + rect.x;
				r.y =detected_dashes_list[p].y + rect.y;
				r.width=detected_dashes_list[p].width;
				r.height=detected_dashes_list[p].height;
				detected_dashes_list_temp.push_back(r);
			}
		detected_dashes_list=find_right_dashes(detected_dashes_list_temp,img,rect);


		return detected_dashes_list;
}

// For the case of two dashes are detected, three text tag will be estimated.
cv::Rect TextTagDetection::restore_tag_by_estimated_dashes(cv::Point estimated_dash_center,cv::Rect text_tag,cv::Mat img,std::vector<cv::Rect> detected_dashes_list)
{

	cv::Point r1, r2;
	r1.x = estimated_dash_center.x-0.5*detected_dashes_list[0].width;
	r1.y = estimated_dash_center.y-0.5*detected_dashes_list[0].height;
	r2.x = detected_dashes_list[0].width;
	r2.y = detected_dashes_list[0].height;

	cv::Rect rect_temp;
	rect_temp.x=r1.x;
	rect_temp.y=r1.y;
	rect_temp.width = r2.x;
	rect_temp.height = r2.y;

	detected_dashes_list.push_back(rect_temp);

	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byCenterX());
//	std::cout<<"detected_dashes_list: "<<detected_dashes_list[0].x<<detected_dashes_list[1].x<<detected_dashes_list[2].x<<std::endl;
	cv:: Rect text_tag_candidate=restore_text_tag_by_three_detected_dashes(detected_dashes_list,text_tag,img);

	return text_tag_candidate;
}
double TextTagDetection::compare_detection_with_template(cv::Rect text_tag, cv::Mat img,std::string package_path)
{
	std::string tag_template_fullname=package_path.append("template.png");
	std::cout<<"tag_template_fullname: "<<tag_template_fullname<<std::endl;
	int match_method=CV_TM_CCOEFF_NORMED;//CV_TM_CCOEFF_NORMED
	cv::Mat template_image = cv::imread(tag_template_fullname,CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat source_image=img(text_tag);
	cv::adaptiveThreshold(template_image, template_image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, -5);
	cv::adaptiveThreshold(source_image, source_image, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 15, -5);

	cv::Size dsize(55,10);
	cv::resize(template_image,template_image,dsize);
	cv::resize(source_image,source_image,dsize);
	double minVal, maxVal,score=0.;
	cv::Point minLoc, maxLoc,matchLoc;
	cv::Mat result;
	matchTemplate(source_image,template_image,result,match_method);

	cv::minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
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
	return score;
}
// this function is to find the best estimated text tag generated by two dashes with computing their similarity with a template.
cv::Rect TextTagDetection::select_best_match_from_three_estimated_dashes(cv::Rect text_tag_l,cv::Rect text_tag_m,cv::Rect text_tag_r,
																					cv::Rect text_tag,cv::Mat img,std::string package_path)
{
	std::string tag_template_fullname=package_path.append("template.png");
	int match_method=CV_TM_CCOEFF_NORMED;//CV_TM_CCOEFF_NORMED
	cv::Rect best_text_tag;
	cv::Mat result_r,result_l,result_m;
	cv::Mat template_image = cv::imread(tag_template_fullname,CV_LOAD_IMAGE_GRAYSCALE);
//	cv::cvtColor(img, img, CV_BGR2GRAY);

	bool left=true,middle=true,right=true;
	if (text_tag_l.width*text_tag_l.height<5)
		left=false;
	if (text_tag_m.width*text_tag_m.height<5 || text_tag_m.width<0.6*text_tag.width)
		middle=false;
	if (text_tag_r.width*text_tag_r.height<5)
		right=false;

	cv::Mat source_image_l=img(text_tag_l);
	cv::Mat source_image_r=img(text_tag_r);
	cv::Mat source_image_m=img(text_tag_m);
	cv::Size dsize(55,10);
	cv::resize(template_image,template_image,dsize);
	cv::resize(source_image_l,source_image_l,dsize);
	cv::resize(source_image_m,source_image_m,dsize);
	cv::resize(source_image_r,source_image_r,dsize);
	double minVal, maxVal,score_l = 0,score_m = 0,score_r = 0;
	cv::Point minLoc, maxLoc,matchLoc;
	matchTemplate(source_image_l,template_image,result_l,match_method);

	cv::minMaxLoc( result_l, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_l=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_l= 1.-maxVal;
	}
	matchTemplate(source_image_m,template_image,result_m,match_method);
	cv::minMaxLoc( result_m, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat() );
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_m=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_m= 1.-maxVal;
	}
	matchTemplate(source_image_r,template_image,result_r,match_method);
	cv::minMaxLoc( result_r, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	if(match_method == cv::TM_SQDIFF || match_method == cv::TM_SQDIFF_NORMED)
	{
		matchLoc = minLoc;
		score_r=minVal;
	}
	else
	{
		matchLoc = maxLoc;
		score_r=1.- maxVal;
	}
	if (!left)
		score_l=10;
	if (!middle)
		score_m=10;
	if (!right)
		score_r=10;

	std::map<double,std::string> map;
	map[score_l]="l";
	map[score_m]="m";
	map[score_r]="r";
	double min1 = std::min(score_m,score_r);
	double minn = std::min(min1,score_l);
	std::string best=map.find(minn) -> second;
//	std::cout<<"best:"<<best<<std::endl;
	if (best=="l")
		best_text_tag=text_tag_l;
	else if (best=="m")
		best_text_tag=text_tag_m;
	else if (best=="r")
		best_text_tag=text_tag_r;
	if (score_l==10 && score_m==10 && score_r==10)
		best_text_tag=text_tag;

	return best_text_tag;
}

//Eliminate some dashes by their shapes or aspect ratio etc and merge intersecting dashes.
std::vector<cv::Rect> TextTagDetection::find_right_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Mat img,cv::Rect rect)
{
	cv::Mat roi = img(rect);
	std::sort(detected_dashes_list.begin(), detected_dashes_list.end(), byArea());
	std::vector <cv::Rect> nonIntersect_dashes_list;
	for(unsigned int i=0; i < detected_dashes_list.size(); i++)
	{
	    bool toAdd = true;
	    cv::Point center = (detected_dashes_list[i].tl()+detected_dashes_list[i].br())*0.5;
	    if (detected_dashes_list[i].height>=detected_dashes_list[i].width)
			toAdd = false;

	    if (double(detected_dashes_list[i].width*detected_dashes_list[i].height)>=0.02*double(rect.width*rect.height))
			toAdd = false;

	    if ((center.y-rect.y)>0.85*roi.rows||(center.y-rect.y)<0.15*roi.rows)
	   			toAdd = false;

	    if (double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)<1.5||
	    			double(detected_dashes_list[i].width)/double(detected_dashes_list[i].height)>4)
	    		toAdd = false;

	    for(unsigned int j=0; j < nonIntersect_dashes_list.size(); j++)
	        if (nonIntersect_dashes_list[j].contains(center))
	        {
	            toAdd = false;
	            break;
	        }

	    if (toAdd)
	        nonIntersect_dashes_list.push_back(detected_dashes_list[i]);
	 }
	std::sort(nonIntersect_dashes_list.begin(), nonIntersect_dashes_list.end(), byCenterX());

	return nonIntersect_dashes_list;
}

//restore text tag by THREE dashes.
cv::Rect TextTagDetection::restore_text_tag_by_three_detected_dashes(std::vector<cv::Rect> detected_dashes_list,cv::Rect text_tag,cv::Mat img)
{

//	cv::Rect text_tag_original=text_tag;
//	cv::Point center0 = (detected_dashes_list[0].tl()+detected_dashes_list[0].br())*0.5;
	cv::Point center1 = (detected_dashes_list[1].tl()+detected_dashes_list[1].br())*0.5;
//	cv::Point center2 = (detected_dashes_list[2].tl()+detected_dashes_list[2].br())*0.5;

	cv::Point tag_center;
	double two_near_dashes_gap = std::fabs( 0.5*(double(detected_dashes_list[2].x) - double(detected_dashes_list[0].x)) );
	tag_center.x=ceil((double(center1.x-text_tag.x)-2./140.*two_near_dashes_gap)+text_tag.x);
	tag_center.y=ceil((97./53.*double(center1.y-text_tag.y)*0.5)+text_tag.y);

	text_tag.width = ceil(690./179.5*two_near_dashes_gap);//two_near_dashes_gap*2 + 127* two_near_dashes_gap/140+ 123* two_near_dashes_gap/140;
	text_tag.height = ceil(129./640.*text_tag.width);
	text_tag.x= center1.x-0.5*text_tag.width;
	text_tag.y= center1.y-6.5/129.*text_tag.height-0.5*text_tag.height;
//	std::cout<<"text_tag: "<<text_tag.x<<" "<<text_tag.y<<" "<<text_tag.width<<" "<<text_tag.height<<std::endl;
	if(text_tag.x<0 || text_tag.y<0 || (text_tag.x+text_tag.width)>img.cols || (text_tag.y+text_tag.height)>img.rows )
	{
		text_tag.x =0;
		text_tag.y =0;
		text_tag.width=1;
		text_tag.height=1;
	}
	return text_tag;
}


////////////////////////////////////////    Viola-Jones Detector  /////////////////////////////////////////////

std::vector<cv::Rect> TextTagDetection::text_tag_detection_with_VJ(cv::Mat image,std::string package_path)
{
	std::vector<cv::Rect> rectangle_list;
//	cv::cvtColor(image, image, CV_RGB2GRAY);
//	cv::equalizeHist(image, image);

	// Load Text cascade (.xml file)
	std::string package_path_temp=package_path;
	std::string text_tags_cascade_xml_file=package_path_temp.append("TextLabelClassifier/haarclassifier_new/cascade.xml");
	cv::CascadeClassifier text_tags_cascade;
	text_tags_cascade.load( text_tags_cascade_xml_file );

	text_tags_cascade.detectMultiScale( image, rectangle_list,1.1,10,0, cv::Size(35, 9),cv::Size());
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
	return rectangle_list;
}

/////////////////////  text tag Detector integrated by VJ Hough transform and dashes detection /////////////////////////

std::vector<cv::Rect> TextTagDetection::text_tag_detection_fine_detection(cv::Mat image,std::string package_path)
{
	std::vector<cv::Rect> rectangle_list,final_rectangle_list;
	cv::Rect retang;
	rectangle_list=text_tag_detection_with_VJ(image,package_path);

	for (std::vector<cv::Rect>::const_iterator r = rectangle_list.begin(); r != rectangle_list.end(); r++)
	{
		cv::Point r1, r2;
		r1.x = (r->x);
		r1.y = (r->y);
		r2.x = (r->x + r->width);
		r2.y = (r->y + r->height);

		retang.x = r1.x;
		retang.y = r1.y;
		retang.width = abs(r2.x - r1.x);
		retang.height = abs(r2.y - r1.y);

		//update detection by Hough line transform
		cv::Mat roi = image(retang);
		cv::Rect rectangle_info=get_rect_with_hough_line_transform(roi);
		cv::Rect rectangle_updated_by_hough_line;

		rectangle_updated_by_hough_line.x=retang.x+rectangle_info.x;
		rectangle_updated_by_hough_line.y=retang.y+rectangle_info.y;
		rectangle_updated_by_hough_line.width=rectangle_info.width;
		rectangle_updated_by_hough_line.height=rectangle_info.height;


		//update detection by dashes detection
		std::vector<cv::Rect> detected_dashes_list=detect_dashes(rectangle_updated_by_hough_line,image);

		cv::Rect rectangle_updated_by_dashes_detection = restore_text_tag_by_detected_dashes(detected_dashes_list,rectangle_updated_by_hough_line,image,package_path);

//		double score = compare_detection_with_template(rectangle_updated_by_dashes_detection,image,package_path);
//
//		std::cout<<"score: "<<score<<std::endl;

		final_rectangle_list.push_back(rectangle_updated_by_dashes_detection);
	}
	return final_rectangle_list;
}





