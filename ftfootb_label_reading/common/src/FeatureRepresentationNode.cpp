#include "ftfootb_label_reading/FeatureRepresentation.h"

int main(int argc, char** argv)
{
	FeatureReprenstation FR;
	if (argc < 1)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}
	FR.help();
	std::string image_filename = argv[1];
	int load = argv[2];
	int classifier = argv[3];
	int feature_name = argv[4];
	cv::Mat text_tag_img = cv::imread(image_filename, 1);

	std::string text_tag=FR.read_text_tag(text_tag_img,load,classifier,feature_name);
	std::cout<<"text tag: " <<text_tag <<std::endl;

}


