#include "lbp.hpp"

using namespace cv;

template <typename _Tp>
void lbp::OLBP_(const Mat& src, Mat& dst)
{
	dst = Mat::zeros(src.rows-2, src.cols-2, CV_8UC1);
	for(int i=1;i<src.rows-1;i++)
	{
		for(int j=1;j<src.cols-1;j++)
		{
			_Tp center = src.at<_Tp>(i,j);
			unsigned char code = 0;
			code |= (src.at<_Tp>(i-1,j-1) > center) << 7;
			code |= (src.at<_Tp>(i-1,j) > center) << 6;
			code |= (src.at<_Tp>(i-1,j+1) > center) << 5;
			code |= (src.at<_Tp>(i,j+1) > center) << 4;
			code |= (src.at<_Tp>(i+1,j+1) > center) << 3;
			code |= (src.at<_Tp>(i+1,j) > center) << 2;
			code |= (src.at<_Tp>(i+1,j-1) > center) << 1;
			code |= (src.at<_Tp>(i,j-1) > center) << 0;
			dst.at<unsigned char>(i-1,j-1) = code;
		}
	}
}
// now the wrapper functions
void lbp::OLBP(const Mat& src, Mat& dst)
{
	switch(src.type())
	{
		case CV_8SC1: OLBP_<char>(src, dst); break;
		case CV_8UC1: OLBP_<unsigned char>(src, dst); break;
		case CV_16SC1: OLBP_<short>(src, dst); break;
		case CV_16UC1: OLBP_<unsigned short>(src, dst); break;
		case CV_32SC1: OLBP_<int>(src, dst); break;
		case CV_32FC1: OLBP_<float>(src, dst); break;
		case CV_64FC1: OLBP_<double>(src, dst); break;
	}
}
// now the Mat return functions
Mat lbp::OLBP(const Mat& src)
{
	Mat dst; OLBP(src, dst); return dst;
}



