/*
 * 3DLocalization.cpp
 *
 *  Created on: Jun 12, 2015
 *      Author: rmb-lw
 */


#include <math.h>
#include <stdio.h>

#include <ftfootb_label_reading/TextTagLocalization.h>

// function1: s1^2*(x1^2+y1^2) + s2^2*(x2^2+y2^2) -2*s1*s2(x1*x2+y1*y2) - w^2
double function1(double s1, double s2,double x1,double y1,double x2,double y2, double w)
{
	return pow(s1,2)*(pow(x1,2)+pow(y1,2)) + pow(s2,2)*(pow(x2,2)+pow(y2,2))-2*s1*s2(x1*x2+y1*y2) - pow(w,2);
}

// function2: s3^2*(x3^2+y3^2) + s4^2*(x4^2+y4^2) -2*s3*s4(x3*x3+y4*y4) - w^2

double function2(double s3, double s4,double x4,double y4,double x3,double y3, double w)
{
	return pow(s3,2)*(pow(x3,2)+pow(y3,2)) + pow(s4,2)*(pow(x4,2)+pow(y4,2))-2*s3*s4(x3*x4+y3*y4) - pow(w,2);
}

// function3: s1^2*(x1^2+y1^2) + s4^2*(x4^2+y4^2) -2*s1*s4(x1*x1+y4*y4) - h^2

double function3(double s1, double s4,double x4,double y4,double x1,double y1, double h)
{
	return pow(s1,2)*(pow(x1,2)+pow(y1,2)) + pow(s4,2)*(pow(x4,2)+pow(y4,2))-2*s1*s4(x1*x4+y1*y4) - pow(h,2);
}

// function4: s3^2*(x3^2+y3^2) + s2^2*(x2^2+y2^2) -2*s3*s2(x3*x3+y2*y2) - h^2

double function4(double s3, double s2,double x2,double y2,double x3,double y3, double h)
{
	return pow(s3,2)*(pow(x3,2)+pow(y3,2)) + pow(s2,2)*(pow(x2,2)+pow(y2,2))-2*s3*s2(x3*x2+y3*y2) - pow(h,2);
}

// df1/ds1 = 2*(x1^2+y1^2)*s1 -2*s2(x1*x2+y1*y2)
double f1s1(double s1, double s2, double x1, double y1, double x2, double y2)
{
	return 2*(pow(x1,2)+pow(y1,2))*s1 -2*s2(x1*x2+y1*y2);
}

// df1/ds2 = 2*(x2^2+y2^2)*s2 -2*s1(x1*x2+y1*y2)
double f1s2(double s1, double s2, double x1, double y1, double x2, double y2)
{
	return 2*(pow(x2,2)+pow(y2,2))*s2 -2*s1(x1*x2+y1*y2);
}

// df2/ds3 = 2*(x3^2+y3^2)*s3 -2*s4(x3*x4+y3*y4)
double f2s3(double s4, double s3, double x3, double y3, double x4, double y4)
{
	return 2*(pow(x3,2)+pow(y3,2))*s3 -2*s4(x3*x4+y3*y4);
}

// df2/ds4 = 2*(x4^2+y4^2)*s4-2*s3(x3*x4+y3*y4)
double f2s4(double s3, double s4, double x3, double y3, double x4, double y4)
{
	return 2*(pow(x4,2)+pow(y4,2))*s4 -2*s3(x3*x4+y3*y4);
}

// df2/ds3 = 2*(x3^2+y3^2)*s3 -2*s4(x3*x4+y3*y4)
double f3s1(double s4, double s1, double x1, double y1, double x4, double y4)
{
	return 2*(pow(x1,2)+pow(y1,2))*s1 -2*s4(x1*x4+y1*y4);
}

// df2/ds4 = 2*(x4^2+y4^2)*s4-2*s3(x3*x4+y3*y4)
double f3s4(double s1, double s4, double x1, double y1, double x4, double y4)
{
	return 2*(pow(x4,2)+pow(y4,2))*s4 -2*s1(x1*x4+y1*y4);
}

// df2/ds3 = 2*(x3^2+y3^2)*s3 -2*s4(x3*x4+y3*y4)
double f4s3(double s4, double s3, double x3, double y3, double x4, double y4)
{
	return 2*(pow(x3,2)+pow(y3,2))*s3 -2*s4(x4*y3+y3*y4);
}

// df2/ds4 = 2*(x4^2+y4^2)*s4-2*s3(x3*x4+y3*y4)
double f4s2(double s2, double s3, double x3, double y3, double x2, double y2)
{
	return 2*(pow(x2,2)+pow(y2,2))*s2 -2*s3(x3*x2+y3*y2);
}

void jacobi(double& j11, double& j12, double& j13,double& j14,
			double& j21, double& j22, double& j23,double& j24,
			double& j31, double& j32, double& j33,double& j34,
			double& j41, double& j42, double& j43,double& j44,
			double x1, double y1,double x2, double y2,double x3, double y3,double x4, double y4,
			double s1, double s2, double s3, double s4)
{
	j11 = f1s1(s1, s2, x1, y1, x2, y2);
	j12 = f1s2(s1, s2, x1, y1, x2, y2);
	j13 = 0;
	j14 = 0;
	j21 = 0;
	j22 = 0;
	j23 = f2s3(s4, s3, x3, y3, x4, y4);
	j23 = f2s4(s4, s3, x3, y3, x4, y4);
	j31 = f3s1(s4, x1, x2);
	*d = f2x2(x1, x2);
}

