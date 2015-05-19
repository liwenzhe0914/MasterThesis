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
	int match_method = 5;
	MatchTemplate mt(path);
	cv::Mat img = cv::imread(image_filename, 1);
	//std::cout << "source imagename: " << image_filename << std::endl;
	std::cout << "image size: " << img.cols<<"x"<<img.rows<< std::endl;

	double start_time;
	double time_in_seconds;
	start_time = clock();

	std::string tag_label;
	mt.read_tag(img, tag_label,match_method);
	std::cout << "The text tag reads: " << tag_label << "." << std::endl;

	time_in_seconds = (clock() - start_time) / (double) CLOCKS_PER_SEC;
	std::cout << "[" << time_in_seconds << " s] processing time" << std::endl;

	return 0;
}
