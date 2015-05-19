#include "ftfootb_label_reading/FeatureRepresentation.h"

int main(int argc, char** argv)
{
	FeatureReprenstation FR;
	if (argc < 5)
	{
		std::cout << "error: not enough input parameters!" << std::endl;
		std::cout << "1). test image \n "
					"2). load training data from yml or not (1=yes,0=no) \n"
					"3). classifier 1=KNN 2=train SVM 3= load SVM\n"
					"4). feature_name 1=HOG 2=LBP 3=BRIEF \n"
					"5). single_or_combination: 1=single character 2=letter/number combinations \n"<< std::endl;
		return -1;
	}
	FR.help();
	//
	int load,classifier,feature_name,single_or_combination;
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
	const char* value5 = argv[5];
	std::stringstream strValue5;
	strValue5 << value5;
	strValue5 >> single_or_combination;

	std::string image_filename = argv[1];
	cv::Mat text_tag_img = cv::imread(image_filename, 1);

	std::string text_tag=FR.read_text_tag(text_tag_img,load,classifier,feature_name,single_or_combination);

}


