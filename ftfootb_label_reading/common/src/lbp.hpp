#ifndef LBP_HPP_
#define LBP_HPP_

//! \author philipp <bytefish[at]gmx[dot]de>
//! \copyright BSD, see LICENSE.

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <limits>

using namespace cv;
using namespace std;

namespace lbp {

// templated functions
template <typename _Tp>
void OLBP_(const cv::Mat& src, cv::Mat& dst);

// wrapper functions
void OLBP(const Mat& src, Mat& dst);

// Mat return type functions
Mat OLBP(const Mat& src);

}
#endif
