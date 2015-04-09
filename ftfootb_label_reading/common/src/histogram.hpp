#ifndef HISTOGRAM_HPP_
#define HISTOGRAM_HPP_

//! \author philipp <bytefish[at]gmx[dot]de>
//! \copyright BSD, see LICENSE.

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

using namespace cv;
using namespace std;

namespace lbp {


Mat histogram_(const Mat& src, int minVal=0, int maxVal=255, bool normed=false);

Mat histogram(InputArray _src, int minVal, int maxVal, bool normed);

Mat spatial_histogram(InputArray _src, int numPatterns,int grid_x, int grid_y, bool /*normed*/);
}
#endif
