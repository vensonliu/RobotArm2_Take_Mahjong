#ifndef _RETINEX_HPP_
#define _RETINEX_HPP_

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

void SSR(Mat& inputImg, Mat& outputImg, int kernelSize, int contrast);
void MSR(Mat& inputImg, Mat& outputImg, int* kernelSize, double* weight, int count, int contrast);
void MSRCR(Mat& inputImg, Mat& outputImg, int* kernelSize, double* weight, int count, int contrast);

#endif
