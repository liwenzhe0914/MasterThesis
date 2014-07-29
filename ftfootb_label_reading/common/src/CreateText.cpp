#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <string>
#include <boost/bind.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;
int main(){
Mat img(200, 200, CV_8UC3, Scalar(255,255,255));
namedWindow( "Display window", WINDOW_AUTOSIZE );
CvFont font1 = fontQt("DejaVu Sans Mono", 25, Scalar(0,0,0));
CvFont font2 = fontQt("FreeMono", 25, Scalar(0,0,0),CV_FONT_BOLD);
addText( img, "WM", Point(0,25), font1);
addText( img, "AA", Point(0,60), font1);
addText( img, "RE", Point(0,100), font1);
addText( img, "WM", Point(100,25), font2);
addText( img, "AA", Point(100,60), font2);
addText( img, "RE", Point(100,100), font2);
imshow( "Display window", img );
waitKey(0);
}
