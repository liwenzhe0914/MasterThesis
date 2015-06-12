/*
 * 3DLocalization.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: rmb-lw
 */



#include <ftfootb_label_reading/TextTagLocalization.h>

// function1: (s1*x1 - s2*x2)*(s1*x1 - s2*x2) + (s1*y1 - s2*y2)*(s1*y1 - s2*y2) + (s1*z1 - s2*z2)*(s1*z1 - s2*z2) - w*w
double function1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2, double w)
{
	return (s1*x1 - s2*x2)*(s1*x1 - s2*x2) + (s1*y1 - s2*y2)*(s1*y1 - s2*y2) + (s1*z1 - s2*z2)*(s1*z1 - s2*z2) - w*w;
}

// function2: (s3*x3 - s4*x4)*(s3*x3 - s4*x4) + (s3*y3 - s4*y4)*(s3*y3 - s4*y4) + (s3*z3 - s4*z4)*(s3*z3 - s4*z4) - w*w

double function2(double s3, double s4,double x3,double y3,double z3, double x4,double y4, double z4, double w)
{
	return (s3*x3 - s4*x4)*(s3*x3 - s4*x4) + (s3*y3 - s4*y4)*(s3*y3 - s4*y4) + (s3*z3 - s4*z4)*(s3*z3 - s4*z4) - w*w;
}

// function3: (s1*x1 - s4*x4)*(s1*x1 - s4*x4) + (s1*y1 - s4*y4)*(s1*y1 - s4*y4) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h

double function3(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4, double h)
{
	return (s1*x1 - s4*x4)*(s1*x1 - s4*x4) + (s1*y1 - s4*y4)*(s1*y1 - s4*y4) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h;
}

// function4: (s2*x2 - s3*x3)*(s2*x2 - s3*x3) + (s2*y2 - s3*y3)*(s2*y2 - s3*y3) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h

double function4(double s2, double s3, double x2, double y2, double z2, double x3, double y3, double z3, double h)
{
	return (s2*x2 - s3*x3)*(s2*x2 - s3*x3) + (s2*y2 - s2*y2)*(s3*y3 - s3*y3) + (s2*z2 - s2*z2)*(s3*z3 - s3*z3) - h*h;
}

// function5: s1*(a*x1 + b*y1 + c*z1) + d
double function5(double s1, double x1, double y1, double z1, double a, double b, double c, double d)
{
	return s1*(a*x1 + b*y1 + c*z1) + d;
}

double function6(double s2, double x2, double y2, double z2, double a, double b, double c, double d)
{
	return s2*(a*x2 + b*y2 + c*z2) + d;
}

double function7(double s3, double x3, double y3, double z3, double a, double b, double c, double d)
{
	return s3*(a*x3 + b*y3 + c*z3) + d;
}

double function8(double s4, double x4, double y4, double z4, double a, double b, double c, double d)
{
	return s4*(a*x4 + b*y4 + c*z4) + d;
}

// df1/ds1
double f1s1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2)
{
	return 2*x1*x1*s1 - 2*s2*x1*x2 + 2*y1*y1*s1 - 2*s2*y1*y2 + 2*z1*z1*s1 - 2*s2*z1*z2;
}

// df1/ds2
double f1s2(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2)
{
	return 2*x2*x2*s2 - 2*s1*x1*x2 + 2*y2*y2*s2 - 2*s2*y1*y2 + 2*z2*z2*s2 - 2*s1*z1*z2;
}

// df2/ds3
double f2s3(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4)
{
	return 2*x3*x3*s3 - 2*s4*x4*x3 + 2*y3*y3*s3 - 2*s4*y4*y3 + 2*z3*z3*s3 - 2*s4*z4*z3;
}

// df2/ds4
double f2s4(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4)
{
	return 2*x4*x4*s4 - 2*s3*x3*x4 + 2*y4*y4*s4 - 2*s3*y4*y3 + 2*z4*z4*s4 - 2*s3*z4*z3;
}

// df3/ds3
double f3s1(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4)
{
	return 2*x1*x1*s1 - 2*s4*x1*x4 + 2*y1*y1*s1 - 2*s4*y4*y1 + 2*z1*z1*s1 - 2*s4*z4*z1;
}

// df3/ds4
double f3s4(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4)
{
	return 2*x4*x4*s4 - 2*s1*x1*x4 + 2*y4*y4*s4 - 2*s1*y4*y1 + 2*z4*z4*s4 - 2*s1*z4*z1;
}

// df4/ds2
double f4s2(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3)
{
	return 2*x2*x2*s2 - 2*s3*x2*x3 + 2*y2*y2*s2 - 2*s3*y3*y2 + 2*z2*z2*s2 - 2*s3*z2*z3;
}

// df4/ds3
double f4s3(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3)
{
	return 2*x3*x3*s3 - 2*s2*x2*x3 + 2*y3*y3*s3 - 2*s2*y3*y2 + 2*z3*z3*s3 - 2*s2*z2*z3;
}

double f5s1(double a, double b,double c, double x1, double y1, double z1)
{
	return a*x1+b*y1+c*z1;
}
double f5a(double s1, double x1)
{
	return s1*x1;
}

double f5b(double s1, double y1)
{
	return s1*y1;
}

double f5c(double s1, double z1)
{
	return s1*z1;
}

double f5d()
{
	return 1.;
}

double f6s2(double a, double b,double c, double x2, double y2, double z2)
{
	return a*x2+b*y2+c*z2;
}
double f6a(double s2, double x2)
{
	return s2*x2;
}

double f6b(double s2, double y2)
{
	return s2*y2;
}

double f6c(double s2, double z2)
{
	return s2*z2;
}

double f6d()
{
	return 1.;
}

double f7s3(double a, double b,double c, double x3, double y3, double z3)
{
	return a*x3+b*y3+c*z3;
}
double f7a(double s3, double x3)
{
	return s3*x3;
}

double f7b(double s3, double y3)
{
	return s3*y3;
}

double f7c(double s3, double z3)
{
	return s3*z3;
}

double f7d()
{
	return 1.;
}

double f8s4(double a, double b,double c, double x4, double y4, double z4)
{
	return a*x4+b*y4+c*z4;
}
double f8a(double s4, double x4)
{
	return s4*x4;
}

double f8b(double s4, double y4)
{
	return s4*y4;
}

double f8c(double s4, double z4)
{
	return s4*z4;
}

double f8d()
{
	return 1.;
}

void jacobi(cv::Mat& jacobi_matrix,
			double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
			double s1, double s2, double s3, double s4)
{
	jacobi_matrix.at<uchar>(0,0) = f1s1(s1, s2, x1, y1, z1, x2, y2, z2);
	jacobi_matrix.at<uchar>(0,1) = f1s2(s1, s2, x1, y1, z1, x2, y2, z2);
//	jacobi_matrix.at<uchar>(0,2) = 0;
//	jacobi_matrix.at<uchar>(0,3) = 0;
//	jacobi_matrix.at<uchar>(0,4) = 0;
//	jacobi_matrix.at<uchar>(0,5) = 0;
//	jacobi_matrix.at<uchar>(0,6) = 0;
//	jacobi_matrix.at<uchar>(0,7) = 0;
	jacobi_matrix.at<uchar>(1,2) = f2s3(s3, s4, x3, y3, z3, x4, y4, z4);
	jacobi_matrix.at<uchar>(1,3) = f2s4(s3, s4, x3, y3, z3, x4, y4, z4);
	jacobi_matrix.at<uchar>(2,0) = f2s3(s3, s4, x3, y3, z3, x4, y4, z4);
	jacobi_matrix.at<uchar>(1,3) = f2s4(s3, s4, x3, y3, z3, x4, y4, z4);



}

