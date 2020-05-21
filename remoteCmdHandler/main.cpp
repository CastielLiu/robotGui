#include "remoteCmdHandler.h"
#include <iostream>

using std::cout;
using std::endl;

void cmdCallback(controlCmd_t cmd)
{
	cout << "xSpeed: " << int(cmd.xSpeed) << "\t zSpeed: " << int(cmd.zSpeed) << endl;
}


int main(int argc,char**argv) 
{
	int myPort = 5050;
	if(argc >1)
		myPort = atoi(argv[1]);
	
	RemoteCmdHandler handler;
	handler.setServerAddr("62.234.114.48",8617);
	//handler.setServerAddr("192.168.0.194",8617);
	handler.setRobotId(myPort);
	handler.bindCallbackFunction(&cmdCallback);
	
	
	if(!handler.start())
		return 0;
	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(200)); 
	}
	handler.stop(); 
	return 0;
}
