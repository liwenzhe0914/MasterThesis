//! \author philipp <bytefish[at]gmx[dot]de>
//! \copyright BSD, see LICENSE.
// modified by Wenzhe Li


#ifndef LBPABDHISTOGRAM_H_
#define LBPABDHISTOGRAM_H_

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <limits>
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/internal.hpp"
#include <set>
#include <vector>

namespace lbp {

// templated functions
template <typename _Tp>
void OLBP_(const cv::Mat& src, cv::Mat& dst);

// wrapper functions
void OLBP(const cv::Mat& src, cv::Mat& dst);

// Mat return type functions
cv::Mat OLBP(const cv::Mat& src);

cv::Mat histogram_(const cv::Mat& src, int minVal=0, int maxVal=255, bool normed=false);

cv::Mat histogram(cv::InputArray _src, int minVal, int maxVal, bool normed);

cv::Mat spatial_histogram(cv::InputArray _src, int numPatterns,int grid_x, int grid_y, bool /*normed*/);
}

#endif /* LBPABDHISTOGRAM_H_ */
