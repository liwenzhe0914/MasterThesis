#include "ftfootb_label_reading/LBPandHistogram.h"


/////////////////////////////////////////////////////// Original LBP /////////////////////////////////////////////////////////
template <typename _Tp>
void lbp::OLBP_(const cv::Mat& src, cv::Mat& dst)
{
	dst = cv::Mat::zeros(src.rows-2, src.cols-2, CV_8UC1);
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
void lbp::OLBP(const cv::Mat& src, cv::Mat& dst)
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
cv::Mat lbp::OLBP(const cv::Mat& src)
{
	cv::Mat dst;
	OLBP(src, dst);
	return dst;
}

///////////////////////////////////////////// LBP Spatial Histogram /////////////////////////////////////////////////////////

cv::Mat lbp::histogram_(const cv::Mat& src, int minVal, int maxVal, bool normed)
{
	cv::Mat result;
    // Establish the number of bins.
    int histSize = maxVal-minVal+1;
    // Set the ranges.
    float range[] = { static_cast<float>(minVal), static_cast<float>(maxVal+1) };

    const float* histRange = {range};
    // calc histogram
    calcHist(&src, 1, 0, cv::Mat(), result, 1, &histSize, &histRange, true, false);
    // normalize
    if(normed) {
        result /= (int)src.total();
    }

    return result.reshape(1,1);
}

cv::Mat lbp::histogram(cv::InputArray _src, int minVal, int maxVal, bool normed)
{
	cv::Mat src = _src.getMat();
    switch (src.type()) {
        case CV_8SC1:
            return histogram_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_8UC1:
            return histogram_(src, minVal, maxVal, normed);
            break;
        case CV_16SC1:
            return histogram_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_16UC1:
            return histogram_(src, minVal, maxVal, normed);
            break;
        case CV_32SC1:
            return histogram_(Mat_<float>(src), minVal, maxVal, normed);
            break;
        case CV_32FC1:
            return histogram_(src, minVal, maxVal, normed);
            break;
        default:
            CV_Error(CV_StsUnmatchedFormats, "This type is not implemented yet."); break;
    }
    return cv::Mat();
}

cv::Mat lbp::spatial_histogram(cv::InputArray _src, int numPatterns, int grid_x, int grid_y, bool /*normed*/)
{
	cv::Mat src = _src.getMat();
    // calculate LBP patch size
    int width = src.cols/grid_x;
    int height = src.rows/grid_y;
    // allocate memory for the spatial histogram
    cv::Mat result = cv::Mat::zeros(grid_x * grid_y, numPatterns, CV_32FC1);
    // return matrix with zeros if no data was given
    if(src.empty())
        return result.reshape(1,1);
    // initial result_row
    int resultRowIdx = 0;
    // iterate through grid
    for(int i = 0; i < grid_y; i++) {
        for(int j = 0; j < grid_x; j++) {
        	cv::Mat src_cell = Mat(src, cv::Range(i*height,(i+1)*height), Range(j*width,(j+1)*width));
        	cv:: Mat cell_hist = histogram(src_cell, 0, (numPatterns-1), true);
            // copy to the result matrix
        	cv::Mat result_row = result.row(resultRowIdx);
            cell_hist.reshape(1,1).convertTo(result_row, CV_32FC1);
            // increase row count in result matrix
            resultRowIdx++;
        }
    }
    // return result as reshaped feature vector
    return result.reshape(1,1);
}
