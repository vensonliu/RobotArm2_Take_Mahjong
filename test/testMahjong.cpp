#include <opencv2/opencv.hpp>
#include <iostream>

#include "mahjong_model.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	mahjong_model ann("../assets/model/");
	string result;

	if(!ann.success)
		return -1;

	Mat pic = imread("./P0.bmp", IMREAD_COLOR);

	result = ann.mahjong_result(pic);
	cout << "result is: " << result << endl;

	return 0;
}
