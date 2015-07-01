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
#include <tf/tf.h>

#include "ftfootb_label_reading/TextTagDetection.h"

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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// new version

	// x-version of 4 ray equations
	inline double fi_rx(double Xi, double Zi, double xi)
	{
		return -Xi + xi*Zi;
	}
	// y-version of 4 ray equations
	inline double fi_ry(double Yi, double Zi, double yi)
	{
		return -Yi + yi*Zi;
	}
	// 4 side length equations
	inline double fi_sl(double Xi1, double Yi1, double Zi1, double Xi2, double Yi2, double Zi2, double side_length)
	{
		return (Xi2-Xi1)*(Xi2-Xi1) + (Yi2-Yi1)*(Yi2-Yi1) + (Zi2-Zi1)*(Zi2-Zi1) - side_length*side_length;
	}

	// optimization function f(u)
	// unknowns u: [X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4]'   (column vector)
	// parameters p: [x1, y1, x2, y2, x3, y3, x4, y4, w, h]'		(w=metric width, h=metric height of rectangle)
	// fu: multi-dimensional optimization function f(u)  (column vector)
	void f(const cv::Mat& u, const cv::Mat& p, cv::Mat& fu)
	{
		fu = cv::Mat::zeros(u.rows, 1, CV_64FC1);
		for (int i=0; i<4; ++i)		// 4 ray equations, x-version
			fu.at<double>(i) = fi_rx(u.at<double>(3*i), u.at<double>(3*i+2), p.at<double>(2*i));
		for (int i=0; i<4; ++i)		// 4 ray equations, y-version
			fu.at<double>(4+i) = fi_ry(u.at<double>(3*i+1), u.at<double>(3*i+2), p.at<double>(2*i+1));
		for (int i=0; i<2; ++i)		// 2 side length equations, height-version
			fu.at<double>(8+i) = fi_sl(u.at<double>(6*i), u.at<double>(6*i+1), u.at<double>(6*i+2), u.at<double>(6*i+3), u.at<double>(6*i+4), u.at<double>(6*i+5), p.at<double>(9));
		for (int i=0; i<2; ++i)		// 2 side length equations, width-version
			fu.at<double>(10+i) = fi_sl(u.at<double>((6*i+3)%u.rows), u.at<double>((6*i+4)%u.rows), u.at<double>((6*i+5)%u.rows), u.at<double>((6*i+6)%u.rows), u.at<double>((6*i+7)%u.rows), u.at<double>((6*i+8)%u.rows), p.at<double>(8));

//		std::cout << "fu:\n" ;
//		for (int i=0; i<fu.rows; ++i)
//			std::cout << fu.at<double>(i) << "\t";
//		std::cout << std::endl;
	}

	// Jacobian of optimization function f(u)
	// unknowns u: [X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4]'   (column vector)
	// parameters p: [x1, y1, x2, y2, x3, y3, x4, y4, w, h]'		(w=metric width, h=metric height of rectangle)
	// Ju: Jacobian J(u)  (2d matrix, row i contains all u-derivatives of fi(u))
	void J(const cv::Mat& u, const cv::Mat& p, cv::Mat& Ju)
	{
		Ju = cv::Mat::zeros(u.rows, u.rows, CV_64FC1);
		for (int i=0; i<4; ++i)		// 4 ray equations, x-version
		{
			Ju.at<double>(i,3*i)   = -1.0;
			Ju.at<double>(i,3*i+2) = p.at<double>(2*i);
		}
		for (int i=0; i<4; ++i)		// 4 ray equations, y-version
		{
			Ju.at<double>(4+i,3*i+1) = -1.0;
			Ju.at<double>(4+i,3*i+2) = p.at<double>(2*i+1);
		}
		for (int i=0; i<2; ++i)		// 2 side length equations, height-version
		{
			Ju.at<double>(8+i,6*i)   = -2*(u.at<double>(6*i+3)-u.at<double>(6*i));
			Ju.at<double>(8+i,6*i+1) = -2*(u.at<double>(6*i+4)-u.at<double>(6*i+1));
			Ju.at<double>(8+i,6*i+2) = -2*(u.at<double>(6*i+5)-u.at<double>(6*i+2));
			Ju.at<double>(8+i,6*i+3) = 2*(u.at<double>(6*i+3)-u.at<double>(6*i));
			Ju.at<double>(8+i,6*i+4) = 2*(u.at<double>(6*i+4)-u.at<double>(6*i+1));
			Ju.at<double>(8+i,6*i+5) = 2*(u.at<double>(6*i+5)-u.at<double>(6*i+2));
		}
		for (int i=0; i<2; ++i)		// 2 side length equations, width-version
		{
			Ju.at<double>(10+i, (6*i+3)%u.rows) = -2*(u.at<double>((6*i+6)%u.rows)-u.at<double>((6*i+3)%u.rows));
			Ju.at<double>(10+i, (6*i+4)%u.rows) = -2*(u.at<double>((6*i+7)%u.rows)-u.at<double>((6*i+4)%u.rows));
			Ju.at<double>(10+i, (6*i+5)%u.rows) = -2*(u.at<double>((6*i+8)%u.rows)-u.at<double>((6*i+5)%u.rows));
			Ju.at<double>(10+i, (6*i+6)%u.rows) = 2*(u.at<double>((6*i+6)%u.rows)-u.at<double>((6*i+3)%u.rows));
			Ju.at<double>(10+i, (6*i+7)%u.rows) = 2*(u.at<double>((6*i+7)%u.rows)-u.at<double>((6*i+4)%u.rows));
			Ju.at<double>(10+i, (6*i+8)%u.rows) = 2*(u.at<double>((6*i+8)%u.rows)-u.at<double>((6*i+5)%u.rows));
		}

//		std::cout << "Ju:\n" ;
//		for (int i=0; i<Ju.rows; ++i)
//		{
//			for (int j=0; j<Ju.cols; ++j)
//				std::cout << Ju.at<double>(i,j) << "\t";
//			std::cout << std::endl;
//		}
	}

	// Newton solver
	// all vectors in type CV_64FC1
	// unknowns u: [X1, Y1, Z1, X2, Y2, Z2, X3, Y3, Z3, X4, Y4, Z4]'   (column vector, input=initial guess, output=solution)
	// parameters p: [x1, y1, x2, y2, x3, y3, x4, y4, w, h]'		(w=metric width, h=metric height of rectangle)
	void newtonOptimization(cv::Mat& u, const cv::Mat& p)
	{
		double diff = 1e10;
		int iterations = 0;
		while (diff > 1e-3 && iterations < 100)
		{
			cv::Mat fu, Ju, du;
			f(u, p, fu);		// f(u)
			J(u, p, Ju);		// J(u)
			cv::solve(Ju, -fu, du);		// Ju*du = -fu
			u = u + du;			// update
			zCheck(u);
			diff = cv::norm(du, cv::NORM_L2);
			++iterations;

//			std::cout << "diff: " << diff << "\nu: " << std::endl;
//			for (int i=0; i<u.rows; ++i)
//				std::cout << u.at<double>(i) << ", ";
//			std::cout << std::endl;
		}
	}

	// prepares initial guess and parameters
	void prepareNewtonOptimization(const TagDetectionData& detection, const double fx, const double fy, const double cx, const double cy, const double w, const double h, cv::Mat& u, cv::Mat& p)
	{
		// parameters
		p = cv::Mat(2*detection.corners_.size()+2, 1, CV_64FC1);
		for (size_t i=0; i<detection.corners_.size(); ++i)
		{
			p.at<double>(2*i)   = (detection.corners_[i].x-cx)/fx;
			p.at<double>(2*i+1) = (detection.corners_[i].y-cy)/fy;
		}
		p.at<double>(2*detection.corners_.size())   = w;
		p.at<double>(2*detection.corners_.size()+1) = h;

		// initial guess of unknows
		u = cv::Mat(3*detection.corners_.size(), 1, CV_64FC1);
		for (size_t i=0; i<detection.corners_.size(); ++i)
		{
			u.at<double>(3*i)   = p.at<double>(2*i);
			u.at<double>(3*i+1) = p.at<double>(2*i+1);
			u.at<double>(3*i+2) = 1.;
		}
	}

	void zCheck(cv::Mat& u)
	{
		// ensure positive z-coordinates
		for (int i=0; i<4; ++i)
		{
			if (u.at<double>(3*i+2) < 0.)
			{
				for (int j=0; j<3; ++j)
					u.at<double>(3*i+j) *= -1;
			}
		}
	}

	void computeLabelPose(const cv::Mat& u, tf::Vector3& position, tf::Quaternion& orientation)
	{
		// position
		position.setZero();
		for (int l=0; l<4; ++l)
		{
			position.setX(position.x() + u.at<double>(3*l));
			position.setY(position.y() + u.at<double>(3*l+1));
			position.setZ(position.z() + u.at<double>(3*l+2));
		}
		position *= 0.25;

		// orientation
		tf::Vector3 axis_x(u.at<double>(6)-u.at<double>(3), u.at<double>(7)-u.at<double>(4), u.at<double>(8)-u.at<double>(5));	// x-axis = X3-X2
		axis_x.normalize();
		tf::Vector3 axis_y(u.at<double>(3)-u.at<double>(0), u.at<double>(4)-u.at<double>(1), u.at<double>(5)-u.at<double>(2));	// x-axis = X2-X1
		axis_y.normalize();
		tf::Vector3 axis_z = axis_x.cross(axis_y);
		axis_z.normalize();
		tf::Matrix3x3 rotation_matrix(axis_x.x(), axis_y.x(), axis_z.x(), axis_x.y(), axis_y.y(), axis_z.y(), axis_x.z(), axis_y.z(), axis_z.z());
		rotation_matrix.getRotation(orientation);
	}
};



#endif /* TEXT_TAG_LOCALIZATION_H_ */
