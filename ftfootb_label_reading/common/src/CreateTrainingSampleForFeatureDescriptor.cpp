#include "ftfootb_label_reading/CreateTemplate.h"

int main (int argc, char** argv)
{
	std::string namefont = "DejaVuSansMono";
	std::string token = argv[0];
	std::string filename;
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	cv::Mat img(25, 25, CV_8UC3, cv::Scalar(255,255,255));
	CvFont font = fontQt(namefont, 8, cv::Scalar(0,0,0),CV_FONT_BOLD);
	cv::addText(img, token, cv::Point(0,8), font);
	filename = "~/git/care-o-bot/ftfootb/ftfootb_label_reading/common/files/LetterTrainingTemplates/" + token + ".png";
	cv::imshow( "Display window", img );
	cv::imwrite(filename, img, compression_params);
}
