/*
 * TextTagLocalization.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: rmb-lw
 */


#include <nlopt.h>
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
			double s1, double s2, double s3, double s4,
			double a, double b, double c)
{	/*	--------------------------------------------------------------------
		 |	s1		s2		s3		s4		a		b		c		d	|
		--------------------------------------------------------------------
	 * * | f1s1 	f1s2 	0 		0 		0 		0 		0 		0	|
	 * * | 0		0		f2s3	f2s4	0		0		0		0	|
	 * * | f3s1		0		0		f3s4	0		0		0		0	|
	 * * | 0		f4s2	f4s3	0		0		0		0		0	|
	 * * | f5s1		0		0		0		f5a		f5b		f5c		f5d	|
	 * * | 0		f6s2	0		0		f6a		f6b		f6c		f6d	|
	 * * | 0		0		f7s3	0		f7a		f7b		f7c		f7d	|
	 * * | 0		0		0		f8s4	f8a		f8b		f8c		f8d	|
	*/
	jacobi_matrix.at<uchar>(0,0) = f1s1(s1, s2, x1, y1, z1, x2, y2, z2);
	jacobi_matrix.at<uchar>(0,1) = f1s2(s1, s2, x1, y1, z1, x2, y2, z2);

	jacobi_matrix.at<uchar>(1,2) = f2s3(s3, s4, x3, y3, z3, x4, y4, z4);
	jacobi_matrix.at<uchar>(1,3) = f2s4(s3, s4, x3, y3, z3, x4, y4, z4);

	jacobi_matrix.at<uchar>(2,0) = f3s1(s1, s4, x1, y1, z1, x4, y4, z4);
	jacobi_matrix.at<uchar>(2,3) = f3s4(s3, s4, x3, y3, z3, x4, y4, z4);

	jacobi_matrix.at<uchar>(3,1) = f4s2(s2, s3, x2, y2, z2, x3, y3, z3);
	jacobi_matrix.at<uchar>(3,2) = f4s3(s2, s3, x2, y2, z2, x3, y3, z3);

	jacobi_matrix.at<uchar>(4,0) = f5s1(a, b, c, x1, y1, z1);
	jacobi_matrix.at<uchar>(4,4) = f5a(s1, x1);
	jacobi_matrix.at<uchar>(4,5) = f5b(s1, y1);
	jacobi_matrix.at<uchar>(4,6) = f5c(s1, z1);
	jacobi_matrix.at<uchar>(4,7) = f5d();

	jacobi_matrix.at<uchar>(5,1) = f6s2(a, b, c, x2, y2, z2);
	jacobi_matrix.at<uchar>(5,4) = f6a(s2, x2);
	jacobi_matrix.at<uchar>(5,5) = f6b(s2, y2);
	jacobi_matrix.at<uchar>(5,6) = f6c(s2, z2);
	jacobi_matrix.at<uchar>(5,7) = f6d();

	jacobi_matrix.at<uchar>(6,2) = f7s3(a, b, c, x3, y3, z3);
	jacobi_matrix.at<uchar>(6,4) = f7a(s3, x3);
	jacobi_matrix.at<uchar>(6,5) = f7b(s3, y3);
	jacobi_matrix.at<uchar>(6,6) = f7c(s3, z3);
	jacobi_matrix.at<uchar>(6,7) = f7d();

	jacobi_matrix.at<uchar>(7,3) = f8s4(a, b, c, x4, y4, z4);
	jacobi_matrix.at<uchar>(7,4) = f8a(s4, x4);
	jacobi_matrix.at<uchar>(7,5) = f8b(s4, y4);
	jacobi_matrix.at<uchar>(7,6) = f8c(s4, z4);
	jacobi_matrix.at<uchar>(7,7) = f8d();
}

cv::Mat delta(const cv::Mat jacobi_matrix, double& ds1, double& ds2,double& ds3,double& ds4, double& da, double& db, double& dc, double& dd,
			double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
			double s1, double s2, double s3, double s4, double a, double b, double c, double d ,double w, double h)
{
	cv::Mat jacobi_matrix_inverse = jacobi_matrix.inv();
	cv::Mat function_mat;
	function_mat.at<uchar>(0,0) = function1(s1,s2,x1,y1,z1,x2,y2,z2,w);
	function_mat.at<uchar>(0,1) = function2(s3,s4,x3,y3,z3,x4,y4,z4,w);
	function_mat.at<uchar>(0,2) = function3(s1,s4,x1,y1,z1,x4,y4,z4,h);
	function_mat.at<uchar>(0,3) = function4(s2,s3,x2,y2,z2,x3,y3,z3,h);
	function_mat.at<uchar>(0,4) = function5(s1,x1,y1,z1,a,b,c,d);
	function_mat.at<uchar>(0,5) = function6(s2,x2,y2,z2,a,b,c,d);
	function_mat.at<uchar>(0,6) = function7(s3,x3,y3,z3,a,b,c,d);
	function_mat.at<uchar>(0,7) = function8(s4,x4,y4,z4,a,b,c,d);

	cv::Mat delta_mat = -1.* jacobi_matrix_inverse.dot(function_mat);

	return delta_mat;
}

void TextTagLocalization(double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4)
{

	 while(1)
	 {

	 }
}
