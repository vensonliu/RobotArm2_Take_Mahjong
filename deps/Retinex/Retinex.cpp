#include "Retinex.hpp"
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

//#define SSR_DEBUG

#ifdef SSR_DEBUG
void print(Mat& input)
{
	for(int row = 0; row < input.rows; row++)
		for(int col = 0; col < input.cols; col++)
			for(int channel = 0; channel < 3; channel++)
                cout << "[" << row << ", " << col << ", " << channel << "]: " << input.at<Vec3f>(row, col)[channel] << endl;
}
#endif // SSR_DEBUG


void normalize_32FC3(Mat& input, Mat& output, float max[3], float min[3])
{
	for(int row = 0; row < input.rows; row++)
		for(int col = 0; col < input.cols; col++)
			for(int channel = 0; channel < 3; channel++)
            {
                float value = input.at<Vec3f>(row, col)[channel];
                if(value < min[channel])
                    value = min[channel];
                else if(value > max[channel])
                    value = max[channel];
				output.at<Vec3f>(row, col)[channel] = (value - min[channel]) / (max[channel] - min[channel]);
            }
}

void subIllumination_32FC3(Mat& input, Mat& illumination, Mat& output)
{
    Mat input_log = input.clone();
    Mat illumination_log = illumination.clone();

#ifdef SSR_DEBUG
	for(int row = 0; row < output.rows; row++)
		for(int col = 0; col < output.cols; col++)
			for(int channel = 0; channel < 3; channel++)
            {
                if(input.at<Vec3f>(row, col)[channel] < 1 ||
                   illumination.at<Vec3f>(row, col)[channel] < 1)
                {
                    cout << "SSR_DEBUG(input<1): [" << row << ", " << col << ", " << channel << "]: "
                        << input.at<Vec3f>(row, col)[channel] << ", "
                        << illumination.at<Vec3f>(row, col)[channel] << endl;
                }
            }
#endif // SSR_DEBUG
    cv::log(input, input_log);
    cv::log(illumination, illumination_log);

	for(int row = 0; row < output.rows; row++)
		for(int col = 0; col < output.cols; col++)
			for(int channel = 0; channel < 3; channel++)
			    output.at<Vec3f>(row, col)[channel] = input_log.at<Vec3f>(row, col)[channel] - illumination_log.at<Vec3f>(row, col)[channel];
}

void minMax_StdDev(Mat& input, float min[3], float max[3], int contrast)
{
    Scalar mean, stddev;
    for(int channel = 0; channel < 3; channel++)
    {
        cv::meanStdDev(input, mean, stddev);
        max[channel] = mean.val[channel] + contrast * stddev.val[channel];
        min[channel] = mean.val[channel] - contrast * stddev.val[channel];
    }
}

void SSR(Mat& inputImg, Mat& outputImg, int kernelSize, int contrast)
{
	if(inputImg.type() != CV_8UC3)
	{
		cout << "input image need to be 8UC3" << endl;
		return;
	}

    Mat input, illumination, reflectance_log;

    inputImg.convertTo(input, CV_32FC3, 1/255.0, 1);
    input.convertTo(illumination, CV_32FC3);
    input.convertTo(reflectance_log, CV_32FC3);
    input.convertTo(outputImg, CV_32FC3);

	GaussianBlur(input, illumination, Size(0, 0), kernelSize, 0);
    subIllumination_32FC3(input, illumination, reflectance_log);

    float min[3], max[3];
    minMax_StdDev(reflectance_log, min, max, contrast);

//	float min[3] = {reflectance_log.at<Vec3f>(0, 0)[0], reflectance_log.at<Vec3f>(0, 0)[1], reflectance_log.at<Vec3f>(0, 0)[2]};
//	float max[3] = {reflectance_log.at<Vec3f>(0, 0)[0], reflectance_log.at<Vec3f>(0, 0)[1], reflectance_log.at<Vec3f>(0, 0)[2]};
//
//	for(int row = 0; row < reflectance_log.rows; row++)
//	{
//		for(int col = 0; col < reflectance_log.cols; col++)
//		{
//			for(int channel = 0; channel < 3; channel++)
//			{
//				float value = reflectance_log.at<Vec3f>(row, col)[channel];
//				if(value < min[channel])
//					min[channel] = value;
//				if(value > max[channel])
//					max[channel] = value;
//			}
//		}
//	}
//
    normalize_32FC3(reflectance_log, outputImg, max, min);
    outputImg.convertTo(outputImg, CV_8UC3, 255);
}


void MSR(Mat& inputImg, Mat& outputImg, int* kernelSize, double* weight, int count, int contrast)
{
	if(inputImg.type() != CV_8UC3)
	{
		cout << "input image need to be 8UC3" << endl;
		return;
	}

    if(count <= 0)
    {
        cout << "count can't be <= 0" << endl;
        outputImg = inputImg.clone();
        return;
    }

    Mat input, output;
    Mat mats[count];
    Mat illumination[count];

    inputImg.convertTo(input, CV_32FC3, 1/255.0, 1);
    input.convertTo(output, CV_32FC3);
    input.convertTo(outputImg, CV_32FC3);
    for(int i = 0; i < count; i++)
    {
        input.convertTo(illumination[i], CV_32FC3);
        input.convertTo(mats[i], CV_32FC3);
    }

    for(int i = 0; i < count; i++)
    {
        GaussianBlur(input, illumination[i], Size(0, 0), kernelSize[i], 0);
        subIllumination_32FC3(input, illumination[i], mats[i]);
    }

	for(int row = 0; row < output.rows; row++)
		for(int col = 0; col < output.cols; col++)
			for(int channel = 0; channel < 3; channel++)
			{
			    output.at<Vec3f>(row, col)[channel] = 0;
                for(int countNum = 0; countNum < count; countNum++)
                {
                    output.at<Vec3f>(row, col)[channel] += (mats[countNum].at<Vec3f>(row, col)[channel] + 1) * weight[countNum];
                }
			}

    float min[3], max[3];
    minMax_StdDev(output, min, max, contrast);

//	float min[3] = {output.at<Vec3f>(0, 0)[0], output.at<Vec3f>(0, 0)[1], output.at<Vec3f>(0, 0)[2]};
//	float max[3] = {output.at<Vec3f>(0, 0)[0], output.at<Vec3f>(0, 0)[1], output.at<Vec3f>(0, 0)[2]};
//
//	for(int row = 0; row < output.rows; row++)
//	{
//		for(int col = 0; col < output.cols; col++)
//		{
//			for(int channel = 0; channel < 3; channel++)
//			{
//				float value = output.at<Vec3f>(row, col)[channel];
//				if(value < min[channel])
//					min[channel] = value;
//				if(value > max[channel])
//					max[channel] = value;
//			}
//		}
//	}

    normalize_32FC3(output, outputImg, max, min);
    outputImg.convertTo(outputImg, CV_8UC3, 255);
}

void MSRCR(Mat& inputImg, Mat& outputImg, int* kernelSize, double* weight, int count, int contrast)
{
	if(inputImg.type() != CV_8UC3)
	{
		cout << "input image need to be 8UC3" << endl;
		return;
	}

    if(count <= 0)
    {
        cout << "count can't be <= 0" << endl;
        outputImg = inputImg.clone();
        return;
    }

    Mat input, output;
    Mat mats[count];
    Mat illumination[count];

    inputImg.convertTo(input, CV_32FC3, 1/255.0, 1);
    for(int i = 0; i < count; i++)
    {
        input.convertTo(illumination[i], CV_32FC3);
        input.convertTo(mats[i], CV_32FC3);
    }

    for(int i = 0; i < count; i++)
    {
        GaussianBlur(input, illumination[i], Size(0, 0), kernelSize[i], 0);
        subIllumination_32FC3(input, illumination[i], mats[i]);
    }

    input.convertTo(output, CV_32FC3);
    input.convertTo(outputImg, CV_32FC3);
	for(int row = 0; row < output.rows; row++)
		for(int col = 0; col < output.cols; col++)
			for(int channel = 0; channel < 3; channel++)
			{
			    output.at<Vec3f>(row, col)[channel] = 0;
                for(int countNum = 0; countNum < count; countNum++)
                {
                    output.at<Vec3f>(row, col)[channel] += (mats[countNum].at<Vec3f>(row, col)[channel] + 1) * weight[countNum];
                }
			}

    const int alpha = 125, beta = 46;
	for(int row = 0; row < output.rows; row++)
		for(int col = 0; col < output.cols; col++)
        {
            float sum = output.at<Vec3f>(row, col)[0] + output.at<Vec3f>(row, col)[1] + output.at<Vec3f>(row, col)[2] + 3;
			for(int channel = 0; channel < 3; channel++)
                output.at<Vec3f>(row, col)[channel] = beta * log((alpha * output.at<Vec3f>(row, col)[channel] + 1) / sum);
        }

    float min[3], max[3];
    minMax_StdDev(output, min, max, contrast);

//	float min[3] = {output.at<Vec3f>(0, 0)[0], output.at<Vec3f>(0, 0)[1], output.at<Vec3f>(0, 0)[2]};
//	float max[3] = {output.at<Vec3f>(0, 0)[0], output.at<Vec3f>(0, 0)[1], output.at<Vec3f>(0, 0)[2]};
//
//	for(int row = 0; row < output.rows; row++)
//	{
//		for(int col = 0; col < output.cols; col++)
//		{
//			for(int channel = 0; channel < 3; channel++)
//			{
//				float value = output.at<Vec3f>(row, col)[channel];
//				if(value < min[channel])
//					min[channel] = value;
//				if(value > max[channel])
//					max[channel] = value;
//			}
//		}
//	}

    normalize_32FC3(output, outputImg, max, min);
    outputImg.convertTo(outputImg, CV_8UC3, 255);
}
