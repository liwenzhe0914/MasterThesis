#include "ftfootb_label_reading/FeatureRepresentation.h"

int main(int argc, char** argv)
{
	if (argc < 1)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}

	std::string image_filename = argv[1];
	cv::Mat text_tag = cv::imread(image_filename, 1);
	FeatureReprenstation FR;

}


