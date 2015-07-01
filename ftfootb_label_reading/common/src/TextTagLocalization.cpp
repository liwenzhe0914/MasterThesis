/*
 * TextTagLocalization.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: rmb-lw
 */


#include <ftfootb_label_reading/TextTagLocalization.h>
#include <stdlib.h>

// function1: (s1*x1 - s2*x2)*(s1*x1 - s2*x2) + (s1*y1 - s2*y2)*(s1*y1 - s2*y2) + (s1*z1 - s2*z2)*(s1*z1 - s2*z2) - w*w
double TextTagLocalization::function1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2, double w)
{
	return (s1*x1 - s2*x2)*(s1*x1 - s2*x2) + (s1*y1 - s2*y2)*(s1*y1 - s2*y2) + (s1*z1 - s2*z2)*(s1*z1 - s2*z2) - w*w;
}

// function2: (s3*x3 - s4*x4)*(s3*x3 - s4*x4) + (s3*y3 - s4*y4)*(s3*y3 - s4*y4) + (s3*z3 - s4*z4)*(s3*z3 - s4*z4) - w*w

double TextTagLocalization::function2(double s3, double s4,double x3,double y3,double z3, double x4,double y4, double z4, double w)
{
	return (s3*x3 - s4*x4)*(s3*x3 - s4*x4) + (s3*y3 - s4*y4)*(s3*y3 - s4*y4) + (s3*z3 - s4*z4)*(s3*z3 - s4*z4) - w*w;
}

// function3: (s1*x1 - s4*x4)*(s1*x1 - s4*x4) + (s1*y1 - s4*y4)*(s1*y1 - s4*y4) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h

double TextTagLocalization::function3(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4, double h)
{
	return (s1*x1 - s4*x4)*(s1*x1 - s4*x4) + (s1*y1 - s4*y4)*(s1*y1 - s4*y4) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h;
}

// function4: (s2*x2 - s3*x3)*(s2*x2 - s3*x3) + (s2*y2 - s3*y3)*(s2*y2 - s3*y3) + (s1*z1 - s4*z4)*(s1*z1 - s4*z4) - h*h

double TextTagLocalization::function4(double s2, double s3, double x2, double y2, double z2, double x3, double y3, double z3, double h)
{
	return (s2*x2 - s3*x3)*(s2*x2 - s3*x3) + (s2*y2 - s2*y2)*(s3*y3 - s3*y3) + (s2*z2 - s2*z2)*(s3*z3 - s3*z3) - h*h;
}

// function5: s1*(a*x1 + b*y1 + c*z1) + d
double TextTagLocalization::function5(double s1, double x1, double y1, double z1, double a, double b, double c, double d)
{
	return s1*(a*x1 + b*y1 + c*z1) + d;
}

double TextTagLocalization::function6(double s2, double x2, double y2, double z2, double a, double b, double c, double d)
{
	return s2*(a*x2 + b*y2 + c*z2) + d;
}

double TextTagLocalization::function7(double s3, double x3, double y3, double z3, double a, double b, double c, double d)
{
	return s3*(a*x3 + b*y3 + c*z3) + d;
}

double TextTagLocalization::function8(double s4, double x4, double y4, double z4, double a, double b, double c, double d)
{
	return s4*(a*x4 + b*y4 + c*z4) + d;
}

// df1/ds1
double TextTagLocalization::f1s1(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2)
{
	return 2*x1*x1*s1 - 2*s2*x1*x2 + 2*y1*y1*s1 - 2*s2*y1*y2 + 2*z1*z1*s1 - 2*s2*z1*z2;
}

// df1/ds2
double TextTagLocalization::f1s2(double s1, double s2,double x1,double y1, double z1, double x2, double y2, double z2)
{
	return 2*x2*x2*s2 - 2*s1*x1*x2 + 2*y2*y2*s2 - 2*s2*y1*y2 + 2*z2*z2*s2 - 2*s1*z1*z2;
}

// df2/ds3
double TextTagLocalization::f2s3(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4)
{
	return 2*x3*x3*s3 - 2*s4*x4*x3 + 2*y3*y3*s3 - 2*s4*y4*y3 + 2*z3*z3*s3 - 2*s4*z4*z3;
}

// df2/ds4
double TextTagLocalization::f2s4(double s3, double s4, double x3,double y3, double z3, double x4, double y4, double z4)
{
	return 2*x4*x4*s4 - 2*s3*x3*x4 + 2*y4*y4*s4 - 2*s3*y4*y3 + 2*z4*z4*s4 - 2*s3*z4*z3;
}

// df3/ds3
double TextTagLocalization::f3s1(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4)
{
	return 2*x1*x1*s1 - 2*s4*x1*x4 + 2*y1*y1*s1 - 2*s4*y4*y1 + 2*z1*z1*s1 - 2*s4*z4*z1;
}

// df3/ds4
double TextTagLocalization::f3s4(double s1, double s4, double x1,double y1, double z1, double x4, double y4, double z4)
{
	return 2*x4*x4*s4 - 2*s1*x1*x4 + 2*y4*y4*s4 - 2*s1*y4*y1 + 2*z4*z4*s4 - 2*s1*z4*z1;
}

// df4/ds2
double TextTagLocalization::f4s2(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3)
{
	return 2*x2*x2*s2 - 2*s3*x2*x3 + 2*y2*y2*s2 - 2*s3*y3*y2 + 2*z2*z2*s2 - 2*s3*z2*z3;
}

// df4/ds3
double TextTagLocalization::f4s3(double s2, double s3, double x2,double y2, double z2, double x3, double y3, double z3)
{
	return 2*x3*x3*s3 - 2*s2*x2*x3 + 2*y3*y3*s3 - 2*s2*y3*y2 + 2*z3*z3*s3 - 2*s2*z2*z3;
}

double TextTagLocalization::f5s1(double a, double b,double c, double x1, double y1, double z1)
{
	return a*x1+b*y1+c*z1;
}
double TextTagLocalization::f5a(double s1, double x1)
{
	return s1*x1;
}

double TextTagLocalization::f5b(double s1, double y1)
{
	return s1*y1;
}

double TextTagLocalization::f5c(double s1, double z1)
{
	return s1*z1;
}

double TextTagLocalization::f5d()
{
	return 1.;
}

double TextTagLocalization::f6s2(double a, double b,double c, double x2, double y2, double z2)
{
	return a*x2+b*y2+c*z2;
}
double TextTagLocalization::f6a(double s2, double x2)
{
	return s2*x2;
}

double TextTagLocalization::f6b(double s2, double y2)
{
	return s2*y2;
}

double TextTagLocalization::f6c(double s2, double z2)
{
	return s2*z2;
}

double TextTagLocalization::f6d()
{
	return 1.;
}

double TextTagLocalization::f7s3(double a, double b,double c, double x3, double y3, double z3)
{
	return a*x3+b*y3+c*z3;
}
double TextTagLocalization::f7a(double s3, double x3)
{
	return s3*x3;
}

double TextTagLocalization::f7b(double s3, double y3)
{
	return s3*y3;
}

double TextTagLocalization::f7c(double s3, double z3)
{
	return s3*z3;
}

double TextTagLocalization::f7d()
{
	return 1.;
}

double TextTagLocalization::f8s4(double a, double b,double c, double x4, double y4, double z4)
{
	return a*x4+b*y4+c*z4;
}
double TextTagLocalization::f8a(double s4, double x4)
{
	return s4*x4;
}

double TextTagLocalization::f8b(double s4, double y4)
{
	return s4*y4;
}

double TextTagLocalization::f8c(double s4, double z4)
{
	return s4*z4;
}

double TextTagLocalization::f8d()
{
	return 1.;
}

cv::Mat TextTagLocalization::jacobi(double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
			double s1, double s2, double s3, double s4,
			double a, double b, double c)
{
	std::cout<<"Start computing jacobi matrix."<<std::endl;
	/*	--------------------------------------------------------------------
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
	cv::Mat jacobi_matrix(8,8,CV_64F);
	double value = 2*x1*x1*s1 - 2*s2*x1*x2 + 2*y1*y1*s1 - 2*s2*y1*y2 + 2*z1*z1*s1 - 2*s2*z1*z2;
	std::cout<<"value: "<< typeid(value).name() <<"\t"<<value<<std::endl;
	jacobi_matrix.at<double>(0,0) = f1s1(s1, s2, x1, y1, z1, x2, y2, z2);
	std::cout<<"jacobi_matrix.at<double>(0,0) :" <<jacobi_matrix.at<double>(0,0)<<std::endl;
	jacobi_matrix.at<double>(0,1) = f1s2(s1, s2, x1, y1, z1, x2, y2, z2);

	jacobi_matrix.at<double>(1,2) = f2s3(s3, s4, x3, y3, z3, x4, y4, z4);
	jacobi_matrix.at<double>(1,3) = f2s4(s3, s4, x3, y3, z3, x4, y4, z4);

	jacobi_matrix.at<double>(2,0) = f3s1(s1, s4, x1, y1, z1, x4, y4, z4);
	jacobi_matrix.at<double>(2,3) = f3s4(s3, s4, x3, y3, z3, x4, y4, z4);

	jacobi_matrix.at<double>(3,1) = f4s2(s2, s3, x2, y2, z2, x3, y3, z3);
	jacobi_matrix.at<double>(3,2) = f4s3(s2, s3, x2, y2, z2, x3, y3, z3);

	jacobi_matrix.at<double>(4,0) = f5s1(a, b, c, x1, y1, z1);
	jacobi_matrix.at<double>(4,4) = f5a(s1, x1);
	jacobi_matrix.at<double>(4,5) = f5b(s1, y1);
	jacobi_matrix.at<double>(4,6) = f5c(s1, z1);
	jacobi_matrix.at<double>(4,7) = f5d();

	jacobi_matrix.at<double>(5,1) = f6s2(a, b, c, x2, y2, z2);
	jacobi_matrix.at<double>(5,4) = f6a(s2, x2);
	jacobi_matrix.at<double>(5,5) = f6b(s2, y2);
	jacobi_matrix.at<double>(5,6) = f6c(s2, z2);
	jacobi_matrix.at<double>(5,7) = f6d();

	jacobi_matrix.at<double>(6,2) = f7s3(a, b, c, x3, y3, z3);
	jacobi_matrix.at<double>(6,4) = f7a(s3, x3);
	jacobi_matrix.at<double>(6,5) = f7b(s3, y3);
	jacobi_matrix.at<double>(6,6) = f7c(s3, z3);
	jacobi_matrix.at<double>(6,7) = f7d();

	jacobi_matrix.at<double>(7,3) = f8s4(a, b, c, x4, y4, z4);
	jacobi_matrix.at<double>(7,4) = f8a(s4, x4);
	jacobi_matrix.at<double>(7,5) = f8b(s4, y4);
	jacobi_matrix.at<double>(7,6) = f8c(s4, z4);
	jacobi_matrix.at<double>(7,7) = f8d();

	std::cout<<"Finish computing jacobi matrix."<<std::endl;

	return jacobi_matrix;
}

cv::Mat TextTagLocalization::delta(const cv::Mat jacobi_matrix, double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
			double s1, double s2, double s3, double s4, double a, double b, double c, double d ,double w, double h)
{
	std::cout<<"Start computing delta matrix."<<std::endl;
	std::cout<<"jacobi_matrix: "<<jacobi_matrix<<std::endl;
//	cv::Mat jacobi_matrix_inverse(8,8,CV_64F);
//	jacobi_matrix_inverse = jacobi_matrix.inv();
	cv::Mat function_mat(8,1,CV_64F);

	function_mat.at<double>(0,0) = function1(s1,s2,x1,y1,z1,x2,y2,z2,w);
	function_mat.at<double>(1,0) = function2(s3,s4,x3,y3,z3,x4,y4,z4,w);
	function_mat.at<double>(2,0) = function3(s1,s4,x1,y1,z1,x4,y4,z4,h);
	function_mat.at<double>(3,0) = function4(s2,s3,x2,y2,z2,x3,y3,z3,h);
	function_mat.at<double>(4,0) = function5(s1,x1,y1,z1,a,b,c,d);
	function_mat.at<double>(5,0) = function6(s2,x2,y2,z2,a,b,c,d);
	function_mat.at<double>(6,0) = function7(s3,x3,y3,z3,a,b,c,d);
	function_mat.at<double>(7,0) = function8(s4,x4,y4,z4,a,b,c,d);

	cv::Mat minus_function_mat(8,1,CV_64F);
	minus_function_mat = function_mat.mul(-1.);

	cv::Mat delta_mat_temp(8,1,CV_64F);
	std::cout<<"here."<<std::endl;
	std::cout<<"function_mat: "<<function_mat<<std::endl;

	cv::Mat delta_mat (8,1,CV_64F);

	delta_mat =  jacobi_matrix.inv(cv::DECOMP_LU)*minus_function_mat;

	std::cout<<"Finish computing delta matrix."<<std::endl;
	return delta_mat;
}

void TextTagLocalization::TransformPixel2Coordinates(double &x, double &y, double &z)
{
	double fx = 1165.997, fy = 1167.095, cx = 630.314, cy = 506.779;

	x = (x - cx)/fx;
	y = (y - cy)/fy;
	z = 1.;
}

void TextTagLocalization::NewtonMethod(double x1, double y1,double z1, double x2, double y2, double z2, double x3, double y3, double z3, double x4, double y4, double z4,
					double w, double h, double& s1, double& s2, double& s3, double& s4, double& a, double& b, double& c, double& d)
{
	TransformPixel2Coordinates(x1, y1, z1);
	TransformPixel2Coordinates(x2, y2, z2);
	TransformPixel2Coordinates(x3, y3, z3);
	TransformPixel2Coordinates(x4, y4, z4);

	std::cout<<"staring NewtonMethod."<<std::endl;
	cv::Mat jacobi_matrix (8,8,CV_64F);
	jacobi_matrix = jacobi(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, s1, s2, s3, s4, a, b, c);

	 while(1)
	 {
		 std::cout<<"NewtonMethod: in the loop."<<std::endl;
		 const double threshold_value = 0.1;
		 cv::Mat delta_mat;
		 delta_mat = delta(jacobi_matrix,x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,s1,s2,s3,s4,a,b,c,d,w,h);
		 //add on to get new guess
		 s1 = delta_mat.at<double>(0,0) + s1;
		 s2 = delta_mat.at<double>(1,0) + s2;
		 s3 = delta_mat.at<double>(2,0) + s3;
		 s4 = delta_mat.at<double>(3,0) + s4;
		 a = delta_mat.at<double>(4,0) + a;
		 b = delta_mat.at<double>(5,0) + b;
		 c = delta_mat.at<double>(6,0) + c;
		 d = delta_mat.at<double>(7,0) + d;

		std::cout<<"processing: "<<"s1: "<<s1<<"\t \t s2: "<<s2<<"\t \t s3: "<<s3<<"\t \t s4: "<<s4
					<<"\t \t a: "<<a<<"\t \t b: "<<b<<"\t \t c: "<<c<<"\t \t d: "<<d<<std::endl;

		std::cout<<fabs(function1(s1,s2,x1,y1,z1,x2,y2,z2,w)) <<"\t" <<fabs(function2(s3,s4,x3,y3,z3,x4,y4,z4,w)) <<"\t" << fabs(function3(s1,s4,x1,y1,z1,x4,y4,z4,h)) <<"\t" << fabs(function4(s2,s3,x2,y2,z2,x3,y3,z3,h)) <<"\t" <<
						 fabs(function4(s2,s3,x2,y2,z2,x3,y3,z3,h)) <<"\t" << fabs(function5(s1,x1,y1,z1,a,b,c,d)) <<"\t" << fabs(function6(s2,x2,y2,z2,a,b,c,d)) <<"\t" << fabs(function7(s3,x3,y3,z3,a,b,c,d)) <<"\t" << fabs(function8(s4,x4,y4,z4,a,b,c,d))<<std::endl;

		 if (fabs(function1(s1,s2,x1,y1,z1,x2,y2,z2,w)) <= threshold_value && fabs(function2(s3,s4,x3,y3,z3,x4,y4,z4,w)) <= threshold_value && fabs(function3(s1,s4,x1,y1,z1,x4,y4,z4,h)) <= threshold_value && fabs(function4(s2,s3,x2,y2,z2,x3,y3,z3,h)) <= threshold_value
				 && fabs(function4(s2,s3,x2,y2,z2,x3,y3,z3,h)) <= threshold_value && fabs(function5(s1,x1,y1,z1,a,b,c,d)) <= threshold_value && fabs(function6(s2,x2,y2,z2,a,b,c,d)) <= threshold_value && fabs(function7(s3,x3,y3,z3,a,b,c,d)) <= threshold_value && fabs(function8(s4,x4,y4,z4,a,b,c,d)) <= threshold_value)
		 {
			 std::cout<<"TextTagLocalization: satisfied threshold value. \n";
			 break;
		 }
	 }
}

void TextTagLocalization::TextTagLocalizationWithNewtonMethod(cv::Rect detected_tectangle, double& X1, double& Y1,double& Z1, double& X2, double& Y2, double& Z2, double& X3, double& Y3, double& Z3, double& X4, double& Y4, double& Z4)
{
	double x1= detected_tectangle.x, y1= detected_tectangle.y , z1= 1.;
	double x2= detected_tectangle.x + detected_tectangle.width, y2= detected_tectangle.y, z2= 1.;
	double x3= detected_tectangle.x, y3= detected_tectangle.y + detected_tectangle.height, z3= 1.;
	double x4= detected_tectangle.x + detected_tectangle.width, y4= detected_tectangle.y + detected_tectangle.height, z4= 1.;

	double w = 690., h = 129.;
	// initial guess
	double s1=1.5, s2=1.5, s3=1.5, s4 = 1.5;
	double a, b, c, d = 1.5;

	TransformPixel2Coordinates (x1, y1, z1);
	TransformPixel2Coordinates (x2, y2, z2);
	TransformPixel2Coordinates (x3, y3, z3);
	TransformPixel2Coordinates (x4, y4, z4);

	NewtonMethod(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, w, h, s1, s2, s3, s4, a, b, c, d);

	X1 = s1*x1;
	Y1 = s1*y1;
	Z1 = s1*z1;
	X2 = s2*x2;
	Y2 = s2*y2;
	Z2 = s2*z2;
	X3 = s3*x3;
	Y3 = s3*y3;
	Z3 = s3*z3;
	X4 = s4*x4;
	Y4 = s4*y4;
}



