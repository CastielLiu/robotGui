#include "main.h"

using std::string;
using std::cout;
using std::endl;

//多线程数据转发
/*
	多个线程，每个线程均可收发，
	服务器提供多个接收端口号，其中一个用作注册端口号，即客户端连接服务器时访问的端口号，
	服务器收到注册信息后启动一个新的线程并创建新的socket（此时系统给分配了一个新的端口号）
	将新端口号通过注册socket发送给客户端，客户端收到新端口号后向新端口号发送注册指令，服务器回应后注册成功。 
	客户端注册成功后添加进客户列表，防止多次注册导致启动多余线程。 
	之后该客户端与服务器的新端口号进行数据交互
 */

//静态变量，运行标志，用于中断多线程 
bool Server::run_flag = true; 

Server::Server(int port):
	register_port_(port)
{
}

Server::~Server()
{
}

//初始化socket返回句柄
//ip为本地ip，端口默认为0，由系统自动分配 
//time_out 为阻塞接收等待时长，默认为0 
int Server::initSocket(const int port, const std::string ip, int time_out)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));//init 0
	
	local_addr.sin_port = htons(port);
	local_addr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, ip.c_str(), &local_addr.sin_addr);
	if(convert_ret !=1)
	{
		perror("convert socket ip failed, please check the format!");
		return -1;
	}

	int fd = socket(PF_INET,SOCK_DGRAM , 0);
	if(fd < 0)
	{
		perror("build socket error");
		return -1;
	}
	int udp_opt = 1;
	// 设置地址可复用 
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &udp_opt, sizeof(udp_opt));
	if(time_out)
	{
		struct timeval timeout;
	    timeout.tv_sec = time_out;//秒
	    //timeout.tv_usec = 0;//微秒
	    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
	}
	
	int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
	if(ret < 0)
	{
		std::cout << "udp bind ip: "<< ip << "\t,port: "<< port << " failed!!" << std:: endl;
		return -1;
	}
	return fd;
} 

//打印socket地址以及端口号信息 
void showSocketMsg(const std::string& prefix, int fd)
{
	struct sockaddr_in serverAddr;
	socklen_t server_len; // = sizeof(sockaddr_in);
	//获取socket信息, ip,port.. 
	getsockname(fd,  (struct sockaddr *)&serverAddr, &server_len);
	char ip[16];
	inet_ntop(AF_INET,&serverAddr.sin_addr,ip,server_len);
	cout << prefix << "\t ip: " << ip << "\t port: " << serverAddr.sin_port << endl;
}

//初始化socket并自动分配端口号，返回值为套接字fd
//参数为引用，将被写入分配的端口号 
//此函数初始化socket时不能设置端口号复用功能，同一端口号被多次绑定导致之前绑定的无法使用 
int Server::initSocketAutoAssignPort(uint16_t& port)
{
	struct sockaddr_in local_addr;
	bzero(&local_addr,sizeof(local_addr));//init 0
	local_addr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, "0.0.0.0", &local_addr.sin_addr);
	
	//区间搜索端口号，直到绑定成功 
	for(port=50000;port<60000; ++port)
	{
		local_addr.sin_port = htons(port);
		int fd = socket(PF_INET,SOCK_DGRAM , 0);
		int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
		if(ret == 0)
		{
			cout << "auto assign port: " << port << endl;
			return fd;
		}
	}
}

//接收客户端注册信息的线程 
void Server::receiveRegisterThread()
{
	int register_fd = initSocket(register_port_); //初始化注册socket 
	//showSocketMsg("register sockect ",register_fd);
	
	const int BufLen =20;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(run_flag)
	{
		int len = recvfrom(register_fd, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		if(len <=0 )
			continue;
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc)
			continue;
		if(pkg->type != PkgType_RequestRegister)
			continue;
		
		// 收到客户端请求注册的信息 
		uint16_t clientId = pkg->senderId;
		auto it = clients_.find(clientId);
		if (it != clients_.end()) //查找到目标客户端 ,表明已经注册
		{
			//cout << "client id: " << clientId << "has in map\n";
			transPack_t pkg(PkgType_repeatLogin);
			sendto(register_fd,(char*)&pkg, sizeof(pkg), 0, 
				  (struct sockaddr*)&client_addr, sizeof(sockaddr_in));
			continue; 
		}
			
		cout << "client id:  " << pkg->senderId  << " request register... " << endl;
		
		clientInfo_t client;
		client.addr = client_addr;
		client.connect = false; //此时客户端与服务端还未建立真正的连接 
		clients_[clientId] = client; //新注册的客户端填入map 
		
		//cout << "write clientId: " << clientId << "to map\n";
		 	
		uint16_t new_port;
		
		//为新注册的客户端新创建一个服务套接字
		int server_fd = initSocketAutoAssignPort(new_port);
		//启动接收和转发线程 
		std::thread t(&Server::receiveAndTransThread,this,server_fd, clientId);
		t.detach();
		
		//向客户端回应为其分配的新端口号信息 
		transPack_t pkg(PkgType_ResponseRegister);
		int headerLen = sizeof(transPack_t); 
		pkg.length = 2;
		char *buf = new char[headerLen+pkg.length];
		memcpy(buf, &pkg, headerLen);

		buf[headerLen] = new_port%256;
		buf[headerLen+1] = new_port/256;
		sendto(register_fd, buf, headerLen+pkg.length, 0, 
				(struct sockaddr*)&client_addr, sizeof(sockaddr_in));
	}
	
	delete [] recvbuf;
}

//接收客户端信息并进行转发的线程 
//客户端断开连接后关闭线程 
void Server::receiveAndTransThread(int server_fd, uint16_t clientId)
{
	//首先接收客户端的确认注册消息，
	//确认注册与注册消息类型一致，
	//注册消息由客户端发送到服务器的公用注册端口号 
	//确认注册消息由客户端发送到服务器为该客户端分配的新端口号 
	const int BufLen1 =2*sizeof(transPack_t);
	uint8_t *recvbuf = new uint8_t [BufLen1];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	//配置为非阻塞，并设置超时时间 
	struct timeval timeout;
    timeout.tv_sec = 10;//秒
    //timeout.tv_usec = 0;//微秒
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
	cout << "new thread start to receive msgs..." << endl;
	int len = recvfrom(server_fd, recvbuf, BufLen1,0,(struct sockaddr*)&client_addr, &clientLen);
	if(len <=0 || //接收超时 
	   recvbuf[0] != 0x66 || recvbuf[1] != 0xcc || //包头错误 
	   pkg->type != PkgType_RequestRegister ||  //指令错误 应为确认注册消息包 
	   clientId != pkg->senderId ) //id不匹配 
	{
		removeClient(clientId); //删除用户
		return; 
	} 
	
	std::cout << "received confirm register msg." << std:: endl; 
	
	clients_[clientId].connect = true; //连接成功
	clients_[clientId].addr = client_addr; //写入客户端地址
	clients_[clientId].fd =  server_fd; //将与该用户建立连接的套接字保存 
	//发送注册成功
	transPack_t temp_pkg(PkgType_RegisterOK);

	//发送多次注册成功信号 
	for(int i=0; i<3; ++i)
	{
		sendto(server_fd,(char*)&temp_pkg, sizeof(temp_pkg), 0, (struct sockaddr*)&client_addr, sizeof(sockaddr_in));
		usleep(50000);
	}
	cout << "clientId: "<< clientId << " register ok ^-^ ^-^ ^-^ ^-^ ^-^ ^-^" << endl; 
	 
	delete [] recvbuf;
	const int BufLen2 = 100000;
	recvbuf = new uint8_t [BufLen2];
	const transPack_t *_pkg = (const transPack_t *)recvbuf;
	
	timeout.tv_sec = 1;//秒
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
	while(run_flag && clients_[clientId].connect)
	{
		//此处接收应为非阻塞，超时后检查 run_flag 以及用户的连接状态
		//否则1.程序无法正常退出，2.用户退出后还在等待接收数据 
		int len = recvfrom(server_fd, recvbuf, BufLen2,0,(struct sockaddr*)&client_addr, &clientLen);
		
		if(len <= 0) continue;
		if(_pkg->head[0] != 0x66 || _pkg->head[1] != 0xcc)
			continue;
		
		//std::cout << "received msg, sender id:" << _pkg->senderId << " type: " << int(_pkg->type) << " len:" << len<< std::endl;
		
		if(_pkg->type == PkgType_HeartBeat) //心跳包 
		{
			cout << "received client heartbeat :" << clientId << endl;
			sendto(server_fd,recvbuf, len, 0, (struct sockaddr*)&client_addr, clientLen); //回发给客户端 
			clients_[clientId].lastHeartBeatTime = time(0); //记录客户心跳时间 
		}
		else if(_pkg->type == PkgType_LogOut)
		{
			cout << "client logout...."<< endl;
			clients_[clientId].connect = false;
		}
		else if(_pkg->type == PkgType_AcceptConnect) //被叫用户接受连接 
		{
			uint16_t srcClientId = ((const transPack_t *)recvbuf)->senderId;
			uint16_t dstClientId = ((const transPack_t *)recvbuf)->receiverId;
			clients_[srcClientId].callingID = dstClientId;
			clients_[dstClientId].callingID = srcClientId;
			cout << "connected " << srcClientId << "\t" << dstClientId << endl;
		}
		else if(_pkg->type == PkgType_RefuseConnect)
		{
			uint16_t dstClientId = ((const transPack_t *)recvbuf)->receiverId;
			sendto(clients_[dstClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
		}
		else if(_pkg->type == PkgType_DisConnect)
		{
			uint16_t clientA =  clientId;
			//A方请求断开，清除A中存放的B的Id，并向B发送断开连接 
			uint16_t clientB = clients_[clientId].callingID;
			if(clientB == 0) //clientA没有正在通话的客户 
				continue; 
			clients_[clientA].callingID = 0; //清除A中B的ID 
			transPack_t pkg(PkgType_DisConnect);
    		sendto(clients_[clientB].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[clientB].addr, sizeof(sockaddr_in));
		}
		else if((pkg->type == PkgType_Video) || (pkg->type == PkgType_Audio))
			msgTransmit(recvbuf, len);
		else if(pkg->type == PkgType_ControlCmd || pkg->type == PkgType_RobotState)
			cmdAndStatusTransmit(recvbuf, len);
		else
		{
			
		}
	}
	cout << "delete client : " << clientId;
	removeClient(clientId);
	delete [] recvbuf;
	close(server_fd);
}

void Server::removeClient(uint16_t id)
{
	uint16_t otherId = clients_[id].callingID;
	if(otherId != 0) //必须判断，若此值为0，对其访问将导致插入ID为0的客户。（map访问不存在的键，自动插入） 
		clients_[otherId].callingID = 0; //删除用户前将与之通话的客户的信息置位	
	clients_.erase(id);
}

void Server::run()
{
	std::thread t1 = std::thread(&Server::printThread,this,15);
	t1.detach();
	
	std::thread t2 = std::thread(&Server::heartBeatThread,this);
	t2.detach(); 
	
	//新建接收客户端注册信息的线程 
	std::thread t = std::thread(&Server::receiveRegisterThread,this);
	t.join();
}

void Server::cmdAndStatusTransmit(const uint8_t* buf, int len)
{
	uint16_t srcClientId = ((const transPack_t *)buf)->senderId;
	uint16_t dstClientId = ((const transPack_t *)buf)->receiverId;
	
	if(dstClientId == ROBOT_TEST_ID) //机器人端测试ID
		return; 
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //未查找到目标客户端 
	{
		//向主叫用户回复 ,回复时按原包头修改指令类型后返回
		transPack_t pkg = *(transPack_t*)buf;
		pkg.type =  PkgType_CalledOffline;
		pkg.length = 0;
		
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));
		//cout << "No client : " << dstClientId << endl;
		return;
	}
	sendto(clients_[dstClientId].fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
}


void Server::msgTransmit(const uint8_t* buf, int len)
{
	uint16_t srcClientId = ((const transPack_t *)buf)->senderId;
	uint16_t dstClientId = ((const transPack_t *)buf)->receiverId;
	int type = ((const transPack_t *)buf)->type;
	
	if(dstClientId == 0) //保留ID 
		return; 
	
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //未查找到目标客户端 
	{
		//向主叫用户回复 ,回复时按原包头修改指令类型后返回
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_CalledOffline;
		pkg.length = 0;
		
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));
		//cout << "No client : " << dstClientId << endl;
		return;
	}
	
	//主叫用户信息中的被叫ID已被写入，表明接通成功 
	if(clients_[srcClientId].callingID == dstClientId) 
	{
		int send_len = sendto(clients_[dstClientId].fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
		//cout << "transmitting " << send_len << " bytes\t from:" <<srcClientId <<" to:" <<  dstClientId << " type: " << type <<  endl;
	}
	//callingID 为空，发起呼叫请求 
	else if(clients_[srcClientId].callingID == 0) 
	{
		//向被叫发送请求连接指令 ,按原包头修改指令类型后发送（旨在包含主叫和被叫的信息） 
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_RequestConnect;
		pkg.length = 0;
		
		sendto(clients_[dstClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
	}
	else //被叫正在与其他用户通话
	{
		transPack_t pkg;
		memcpy(&pkg, buf, sizeof(transPack_t));
		pkg.type =  PkgType_CalledBusy;
		sendto(clients_[srcClientId].fd, (char*)&pkg, sizeof(transPack_t), 0, (struct sockaddr*)&clients_[srcClientId].addr, sizeof(sockaddr_in));	
	} 
	
}

//由于遇到过服务器自动断开的问题 
//定时向终端打印数据的线程，保持服务器一直处于唤醒状态
void Server::printThread(int interval)
{
	int i=0;
	while(run_flag)
	{
		sleep(interval);
		printf("this server has been running for %d seconds.\n",interval*(++i));
	}
}

void Server::heartBeatThread()
{
	while(run_flag)
	{
		//cout << "clients size: " << clients_.size() << "\t sending heart beat pkg...\n";
		for(auto client =clients_.begin(); client!= clients_.end(); ++client)
		{
			//cout << "id: " << client->first <<endl;
			//初始情况下客户上次心跳时间为0 
			if(client->second.lastHeartBeatTime ==0)
			{
				//此处修改上次心跳时间，防止有些客户注册后从未心跳。 
				client->second.lastHeartBeatTime = 1; 
				continue;
			} 
			std::time_t diff =  time(0) - client->second.lastHeartBeatTime;
			
			//客户端连接标志 connect复位后，将退出其接收线程，然后自动删除用户
			//若某些用户未能自动删除，手动删除 
			 
			if(diff > heartBeatInterval_ + maxHeartBeatDelay_*10) 
			{
				//删除未能正常删除的用户
				removeClient(client->first);
			}
			else if( diff >heartBeatInterval_ + maxHeartBeatDelay_) 
			{
				//连接置false，等待线程退出后自动删除用户 
				client->second.connect = false;
				cout << "client " << client->first  << "  disconnect." << endl;
			}
			 
		}
		std::this_thread::sleep_for(std::chrono::seconds(heartBeatInterval_)); 
	}
}

//系统中断信号捕获
void sigint_handler(int signal_num)
{
	//std::cout << "signal_num: " << signal_num << std::endl;
	Server::run_flag = false;
	usleep(100000); //预留时间清理线程 
	exit(0);
}

int main(int argc,char** argv)
{
	signal(SIGINT, sigint_handler);
	int port = 8617;
	if(argc > 1)
		port = atoi(argv[1]);
	Server server(port);
	
	server.run();
 
	return 0;
}

// scp * root@aliyun:/root/seu/wendao/remote_driverless_server
