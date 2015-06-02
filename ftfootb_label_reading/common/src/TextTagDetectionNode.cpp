#include "ftfootb_label_reading/TextTagDetection.h"

int main(int argc, char** argv)
{
	if (argc < 1)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}
 std::string path = "/home/rmb-om/git/care-o-bot/ftfootb/ftfootb_label_reading/common/files";
	std::string image_filename = argv[1];

	TextTagDetection ttd;
	std::vector<cv::Rect> detection_list;
	cv::Mat img = cv::imread(image_filename, 1);
//	std::cout << "image size: " << img.cols<<"x"<<img.rows<< std::endl;

	double start_time;
	double time_in_seconds;
	start_time = clock();
	detection_list=ttd.text_tag_detection_fine_detection(img,path);

	cv::Mat img_color = cv::imread(image_filename,CV_LOAD_IMAGE_COLOR);
	for (unsigned int i = 0; i< detection_list.size();i++)
	{
		cv::rectangle(img_color,detection_list[i],cv::Scalar(0,0,255), 3, 8, 0);
	}
	cv::imshow("Final detection",img_color);
	cv::waitKey();
	time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;
}
