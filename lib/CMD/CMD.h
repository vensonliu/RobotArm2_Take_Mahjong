#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <Serial/Serial.h>
#include <iostream>

using namespace cv;
using namespace std;


enum CmdStatus{success, help, error};

class CMD
{
	private:
		CommandLineParser cmdParser;
		string portName;
		enum CmdStatus status;

	public:
		CMD(int argc, char** argv, string keys): cmdParser(argc, argv, keys)
		{
			analysis();
		}

		void analysis();
		string getPortName() const;
		enum CmdStatus getStatus() const;
};
