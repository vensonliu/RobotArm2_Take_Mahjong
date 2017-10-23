#include "Hsv/HSV.h"
#include "Serial/Serial.h"
#include "RobotArm/RobotArm.h"
#include "Camera/Camera.h"
#include "CMD/CMD.h"
#include "mahjong_model.h"

#include <iostream>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <ann.h>
#include <sys/types.h>
#include <dirent.h>


using namespace std;
using namespace cv;

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 480


vector<Mat> SlidingWindow(Mat& armMat, HSV& hsvParam);
bool detect(Mat& input, Mat& mat, Point& coor);

int main(int argc, char** argv)
{
	string keys =
		"{help h        |              | show available parameters}"
		"{@portname port | /dev/ttyACM0 | port to connect to second robotArm}"
		"{list l        |              | list the portname}";

	CMD cmd(argc, argv, keys);
	if(cmd.getStatus() != CmdStatus::success)
		return -1;

	string portName = cmd.getPortName();
	RobotArm RobotArm2;
	Serial serial(portName, 115200);
	
	HSV armHsv_bg("../assets/armHsv_bg.ini", "armHsv_bg", 0);
	Camera armCam(0, "armCam", FRAME_WIDTH, FRAME_HEIGHT);
	char inputKey;	

	vector<Mat> results;
	mahjong_model ann("../assets/model");
	string result;

	//connect serial
	if(serial.connect() < 0)
	{
		cout << portName << "connection error" << endl;
		return -1;
	}

	//loading ann model
	if(!ann.success)
		return -1;
	
	//move robotarm2 to takeThing
	serial.blocking_write(RobotArm2.takeThing());
	if(serial.F3_read(20) != 1)
	{
		cout << "time out when take thing" << endl;
		goto SET_ZERO;
	}

	while(1)
	{
		armCam.read();

		armCam.show();
		inputKey = waitKey(1);
		switch(inputKey)
		{
			case 27:
				armHsv_bg.HSV_save("../assets/armHsv_bg.ini");
				goto SET_ZERO;
			case 'a':
				results = SlidingWindow(armCam.image, armHsv_bg);
				result = ann.mahjong_result(results);
				cout << result << endl;
				break;
		}
	}
	
SET_ZERO:
	serial.blocking_write(RobotArm2.goHome());
	return 0;
}

vector<Mat> SlidingWindow(Mat& armMat, HSV& hsvParam)
{
	Mat gray, hsv, captured;
	vector<Mat> output;
	Point coor;

	cvtColor(armMat, hsv, CV_BGR2HSV);
	HSV_Filter(hsv, gray, hsvParam);

	if(detect(gray, captured, coor))
	{
		imshow("gray", gray);
		imshow("captured", captured);
		waitKey(0);
		for(int y = 0; y < captured.rows - ANN_HEIGHT; y+=10)
		{
			for(int x = 0; x < captured.cols - ANN_WIDTH; x+=10)
			{
				Rect rect(x, y, ANN_WIDTH, ANN_HEIGHT);
				Mat tmp = captured(rect).clone();
				output.push_back(tmp);
			}
		}
	}

	return output;
}

bool detect(Mat& input, Mat& mat, Point& coor)
{
	vector<vector<Point>> contours;
	Rect box;

	Mat srcTemp = input.clone();
	
	int area = 0;

	findContours(srcTemp, contours, RETR_LIST, CV_CHAIN_APPROX_NONE);
	for(unsigned int i = 0; i < contours.size(); i++)
	{
		box = boundingRect(contours[i]);
		if (box.width > 10 && box.height > 10 &&
			box.width < 300 && box.height < 300)
		{
			if(box.width * box.height > area)
			{
				Point center = (box.br() + box.tl()) / 2;
				if (center.x - box.width/2 > 0 &&
					center.x + box.width/2 < srcTemp.rows &&
					center.y - box.height/2 > 0 &&
					center.y + box.height/2 < srcTemp.cols)
				{
					area = box.width * box.height;
					mat = input(box).clone();
					coor = center;
				}
			}
		}
	}
	if(area != 0)
		return true;
	else
		return false;
}
