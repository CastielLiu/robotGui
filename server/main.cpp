#include "main.h"

using std::string;
using std::cout;
using std::endl;

//多线程数据转发
/*
方法1. 建立多个转发线程，接收线程唯一. 每个接收线程独立对应一个数据接收缓冲区，并设立条件变量，当接收进程收到数据并存储缓冲区x中时唤醒线程x进行转发
当前数据接收缓冲区应进行合理选择，正在使用的缓冲区应设置标志位，对标志位进行读写时应加锁
最初可以初始化N个线程，当线程不够使用时再动态增加
 **方法1 用一个线程接收数据多个线程转发数据，一定程度上可以提高效率**
方法2. 多个线程，每个线程均可收发，由于多个绑定同一地址端口对的socket只有第一绑定的可以接收数据，因此应采用不同线程不同端口号的方式
	服务器提供多个接收端口号，其中一个用作注册端口号，即客户端连接服务器时访问的端口号，
	服务器收到注册信息后启动一个新的线程并创建新的socket向客户端发送注册成功信息，（此时系统给分配了一个新的端口号）
	客户端注册成功后添加进客户列表，方式多次注册导致启动多余线程。 
	之后该用户端用服务器分配的端口号进行数据请求
 */

bool Server::run_flag = true; 

Server::Server(int port):
	register_port_(port)
{
	register_fd_ = -1;
}

Server::~Server()
{
	if(register_fd_ != -1)
		close(register_fd_);
}

//初始化socket返回句柄
//ip为本地ip，端口默认为0，由系统自动分配 
int Server::initSocket(const int port, const std::string ip)
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
	
	if(port == 0) //如果port为默认值，不绑定端口，由主机分配 
		return fd;
	
	int ret = bind(fd, (struct sockaddr*)&local_addr,sizeof(local_addr));
	if(ret < 0)
	{
		std::cout << "udp bind ip: "<< ip << "\t,port: "<< port << " failed!!" << std:: endl;
		return -1;
	}
	return fd;
 } 


//接收客户端注册信息的线程 
void Server::receiveRegisterThread()
{
	const int BufLen =20;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(run_flag)
	{
		int len = recvfrom(register_fd_, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc)
			continue;
		if(pkg->type != Register)
			continue;
		
		cout << "receiveRegisterMsg, client id:  " << pkg->senderId << "\t msg len:" << len << endl;
		
		uint16_t clientId = pkg->senderId;
		auto it = clients_.find(clientId);
		if (it != clients_.end()) //查找到目标客户端 ,表明已经注册
			continue; 
		clientInfo_t client;
		client.addr = client_addr;
		client.connect = true;
		clients_[clientId] = client; //新注册的客户端填入map 
		
		//启动接收和转发线程 
		std::thread t(&Server::receiveAndTransThread,this,clientId);
		t.detach();
	}
}

//接收客户端信息并进行转发的线程 
void Server::receiveAndTransThread(uint16_t clientId)
{
	//首先建立新的socket并发送接受注册指令
	//客户端收到接受注册指令后记录服务器的新端口号，并用此端口号进行数据发送
	int fd = initSocket(8618);
	if(fd < 0)
	{
		clients_[clientId].connect = false;
		return;
	}
	
	//发送注册成功
	transPack_t temp_pkg;
	temp_pkg.type = RegisterOK;
	
	//发送多次注册成功信号 
	for(int i=0; i<3; ++i)
	{
		cout << "clientId: "<< clientId << " register ok.  addr " << clients_[clientId].addr.sin_port << endl; 
		sendto(fd,(char*)&temp_pkg, sizeof(temp_pkg), 0, (struct sockaddr*)&clients_[clientId].addr, sizeof(sockaddr_in));
		usleep(300000);
	}
	
	
	const int BufLen =100000;
	uint8_t *recvbuf = new uint8_t [BufLen];
	const transPack_t *pkg = (const transPack_t *)recvbuf;
	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(clients_[clientId].connect)
	{
		int len = recvfrom(fd, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		
		if(pkg->head[0] != 0x66 || pkg->head[1] != 0xcc)
			continue;
		//uint8_t type = pkg.type();
	
		msgTransmit(fd, recvbuf, len);
	}
	delete [] recvbuf;
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

void Server::run()
{
	std::thread t1 = std::thread(&Server::printThread,this,5);
	t1.detach();
	
	register_fd_ = initSocket(register_port_); //初始化注册socket 
	if(register_fd_ < 0)
		return; 
	
	//新建接收客户端注册信息的线程 
	std::thread t = std::thread(&Server::receiveRegisterThread,this);
	t.join();
}

void Server::msgTransmit(int fd, const uint8_t* buf, int len)
{
	uint16_t dstClientId = buf[9]*256 + buf[8];
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //未查找到目标客户端 
	{
		cout << "No client : " << dstClientId << endl;
		return;
	}
	int send_len = sendto(fd, buf, len, 0, (struct sockaddr*)&clients_[dstClientId].addr, sizeof(sockaddr_in));
	cout << "transmitting : " << send_len << " bytes to id: " << dstClientId << "\tport：" << clients_[dstClientId].addr.sin_port << endl;
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
