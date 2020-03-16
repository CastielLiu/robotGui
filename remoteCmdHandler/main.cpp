#include "remoteCmdHandler.h"
#include <iostream>

using std::cout;
using std::endl;

void cmdCallback(controlCmd_t cmd)
{
	cout << "xSpeed: " << int(cmd.xSpeed) << "\t zSpeed: " << int(cmd.zSpeed) << endl;
}


int main()
{
	RemoteCmdHandler handler;
	handler.bindCallbackFunction(&cmdCallback);
	if(!handler.start())
		return 0;
	while(true)
	{
		std::this_thread::sleep_for(std::chrono::seconds(200)); 
	} 
		
	return 0;
}
