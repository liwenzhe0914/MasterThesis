#include "histogram.hpp"
#include <vector>


Mat lbp::histogram_(const Mat& src, int minVal, int maxVal, bool normed)
{
    Mat result;
    // Establish the number of bins.
    int histSize = maxVal-minVal+1;
    // Set the ranges.
    float range[] = { static_cast<float>(minVal), static_cast<float>(maxVal+1) };

    const float* histRange = {range};
    // calc histogram
    calcHist(&src, 1, 0, Mat(), result, 1, &histSize, &histRange, true, false);
    // normalize
    if(normed) {
        result /= (int)src.total();
    }
    cout<<"result: "<<result.reshape(1,1)<<endl;

    return result.reshape(1,1);
}

Mat lbp::histogram(InputArray _src, int minVal, int maxVal, bool normed)
{
    Mat src = _src.getMat();
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
    return Mat();
}

Mat lbp::spatial_histogram(InputArray _src, int numPatterns, int grid_x, int grid_y, bool /*normed*/)
{
    Mat src = _src.getMat();
    // calculate LBP patch size
    int width = src.cols/grid_x;
    int height = src.rows/grid_y;
    // allocate memory for the spatial histogram
    Mat result = Mat::zeros(grid_x * grid_y, numPatterns, CV_32FC1);
    // return matrix with zeros if no data was given
    if(src.empty())
        return result.reshape(1,1);
    // initial result_row
    int resultRowIdx = 0;
    // iterate through grid
    for(int i = 0; i < grid_y; i++) {
        for(int j = 0; j < grid_x; j++) {
            Mat src_cell = Mat(src, Range(i*height,(i+1)*height), Range(j*width,(j+1)*width));
            Mat cell_hist = histogram(src_cell, 0, (numPatterns-1), true);
            // copy to the result matrix
            Mat result_row = result.row(resultRowIdx);
            cell_hist.reshape(1,1).convertTo(result_row, CV_32FC1);
            // increase row count in result matrix
            resultRowIdx++;
        }
    }
    // return result as reshaped feature vector
    return result.reshape(1,1);
}
