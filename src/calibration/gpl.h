#ifndef GPL_H
#define GPL_H

#include <algorithm>
#include <cmath>
#include <opencv2/core/core.hpp>
#include <stdint.h>

namespace px
{

template<class T>
const T clamp(const T& v, const T& a, const T& b)
{
	return std::min(b, std::max(a, v));
}

double hypot3(double x, double y, double z);
float hypot3f(float x, float y, float z);

template<class T>
const T normalizeTheta(const T& theta)
{
	T normTheta = theta;

	while (normTheta < - M_PI)
	{
		normTheta += 2.0 * M_PI;
	}
	while (normTheta > M_PI)
	{
		normTheta -= 2.0 * M_PI;
	}

	return normTheta;
}

double d2r(double deg);
float d2r(float deg);
double r2d(double rad);
float r2d(float rad);

template<class T>
const T square(const T& x)
{
	return x * x;
}

template<class T>
const T random(const T& a, const T& b)
{
	return static_cast<double>(rand()) / RAND_MAX * (b - a) + a;
}

uint64_t timeInMicroseconds(void);

double timeInSeconds(void);

void colorDepthImage(cv::Mat& imgDepth, cv::Mat& imgColoredDepth,
					 float minRange, float maxRange);

bool colormap(const std::string& name, unsigned char idx,
			  float& r, float& g, float& b);

std::vector<cv::Point2i> bresLine(int x0, int y0, int x1, int y1);
std::vector<cv::Point2i> bresCircle(int x0, int y0, int r);

}

#endif
