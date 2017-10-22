#include <CMD.h>

void CMD::analysis()
{
	if(cmdParser.has("help"))
	{
		cmdParser.printMessage();
		status = CmdStatus::help;
		return;
	}

	if(cmdParser.has("list"))
	{
		vector<string> portNames = Serial::getPortNames();
		for(int i = 0; i < (int) portNames.size(); i++)
			cout << portNames[i] << endl;
		status = CmdStatus::help;
		return;
	}

	portName = cmdParser.get<string>("@portname");
	if(!cmdParser.check())
	{
		cmdParser.printErrors();
		status = CmdStatus::error;
		return;
	}
	
	status = CmdStatus::success;
}

string CMD::getPortName() const
{
	return portName;
}

enum CmdStatus CMD::getStatus() const
{
	return status;
}
