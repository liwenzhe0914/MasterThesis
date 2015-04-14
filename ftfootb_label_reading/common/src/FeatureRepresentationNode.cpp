#include "ftfootb_label_reading/FeatureRepresentation.h"

int main(int argc, char** argv)
{
	FeatureReprenstation FR;
	if (argc < 4)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		return -1;
	}
	FR.help();

	int load,classifier,feature_name;
	const char* value2 = argv[2];
	std::stringstream strValue2;
	strValue2 << value2;
	strValue2 >> load;
	const char* value3 = argv[3];
	std::stringstream strValue3;
	strValue3 << value3;
	strValue3 >> classifier;
	const char* value4 = argv[4];
	std::stringstream strValue4;
	strValue4 << value4;
	strValue4 >> feature_name;

	std::string image_filename = argv[1];
	cv::Mat text_tag_img = cv::imread(image_filename, 1);

	std::string text_tag=FR.read_text_tag(text_tag_img,load,classifier,feature_name);
	std::cout<<"text tag: " <<text_tag <<std::endl;

}


