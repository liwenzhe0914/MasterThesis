#include "ftfootb_label_reading/CreateTemplate.h"

int main() {
	//current_path = path();
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);
	wordfile1.open("letters.dat", std::ios::in);
	wordfile2.open("numbers.dat", std::ios::in);
	if (wordfile1.is_open()==false || wordfile2.is_open()==false)
	{
		std::cout << "CreateTemplate::main:Error: Could not open files." << std::endl;
		return -1;
	}
	cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
	namefont = "DejaVuSansMono";
	while (wordfile1 >> letters) {
		list1.push_back(letters);
	}
	while (wordfile2 >> numbers) {
		list2.push_back(numbers);
	}

	for (size_t i=0; i<list1.size(); ++i)
	{
		for (size_t j=0; j<list1.size(); ++j)
		{
			letter_combnations = list1.at( i ) + list1.at( j );
			cv::Mat img(25, 40, CV_8UC3, cv::Scalar(255,255,255));
			CvFont font = fontQt(namefont, 25, cv::Scalar(0,0,0),CV_FONT_BOLD);
			cv::addText( img, letter_combnations, cv::Point(0,25), font);
			filename1 = "/home/damon/MasterThesis/LettersTemplate/" + letter_combnations + ".png";
			cv::imshow( "Display window", img );
			cv::imwrite(filename1, img, compression_params);
		}
	}
	for (size_t m=0; m<list2.size(); ++m)
	{
		for (size_t n=0; n<list2.size(); ++n)
		{
			number_combnations = list2.at( m ) + list2.at( n );
			cv::Mat img(25, 40, CV_8UC3, cv::Scalar(255,255,255));
			cv::namedWindow( "Display window", cv::WINDOW_AUTOSIZE );
			CvFont font = fontQt(namefont, 25, cv::Scalar(0,0,0),CV_FONT_BOLD);
			cv::addText( img, number_combnations, cv::Point(0,25), font);
			filename2 = "/home/damon/MasterThesis/NumbersTemplate/" + number_combnations + ".png";
			cv::imshow( "Display window", img );
			cv::imwrite(filename2, img, compression_params);
		}
	}
	std::cout <<std::endl;
	return 0;
}

