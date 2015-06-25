/*
 * 3DLocaliazation.h
 *
 *  Created on: Jun 12, 2015
 *      Author: rmb-lw
 */

#ifndef TEXT_TAG_LOCALIZATION_H_
#define TEXT_TAG_LOCALIZATION_H_
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "opencv/cv.h"
#include "opencv/highgui.h"
//#include <tf/tf.h>

class TextTagLocalization
{
	public:
	//Transform localize_text_tag();

	cv::Mat jacobi(double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
				double s1, double s2, double s3, double s4, double a, double b, double c);

	cv::Mat delta(const cv::Mat jacobi_matrix, double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
				double s1, double s2, double s3, double s4, double a, double b, double c, double d ,double w, double h);

	void TransformPixel2Coordinates(double &x, double &y, double &z);

	void NewtonMethod(double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
						double w, double h, double& s1, double& s2, double& s3, double& s4, double& a, double& b, double& c, double& d);

	void TextTagLocalizationWithNewtonMethod(cv::Rect detected_tectangle, double& X1, double& Y1,double& Z1, double& X2, double& Y2, double& Z2, double& X3, double& Y3, double& Z3, double& X4, double& Y4, double& Z4);

	// 8 functions:
	double function1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2, double w);
	double function2(double s3, double s4,double x3,double y3,double z3, double x4,double y4, double z4, double w);
	double function3(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4, double h);
	double function4(double s2, double s3, double x2, double y2, double z2, double x3, double y3, double z3, double h);
	double function5(double s1, double x1, double y1, double z1, double a, double b, double c, double d);
	double function6(double s2, double x2, double y2, double z2, double a, double b, double c, double d);
	double function7(double s3, double x3, double y3, double z3, double a, double b, double c, double d);
	double function8(double s4, double x4, double y4, double z4, double a, double b, double c, double d);

	//Partial derivatives
	double f1s1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2);
	double f1s2(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2);
	double f2s3(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4);
	double f2s4(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4);
	double f3s1(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4);
	double f3s4(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4);
	double f4s2(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3);
	double f4s3(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3);
	double f5s1(double a, double b,double c, double x1, double y1, double z1);
	double f5a(double s1, double x1);
	double f5b(double s1, double y1);
	double f5c(double s1, double z1);
	double f5d();
	double f6s2(double a, double b,double c, double x2, double y2, double z2);
	double f6a(double s2, double x2);
	double f6b(double s2, double y2);
	double f6c(double s2, double z2);
	double f6d();
	double f7s3(double a, double b,double c, double x3, double y3, double z3);
	double f7a(double s3, double x3);
	double f7b(double s3, double y3);
	double f7c(double s3, double z3);
	double f7d();
	double f8s4(double a, double b,double c, double x4, double y4, double z4);
	double f8a(double s4, double x4);
	double f8b(double s4, double y4);
	double f8c(double s4, double z4);
	double f8d();

};



#endif /* TEXT_TAG_LOCALIZATION_H_ */
