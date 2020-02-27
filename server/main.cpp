#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <thread>
#include <mutex>
#include <iostream>
#include <map>

#include <signal.h>

using std::string;
using std::cout;
using std::endl;

enum dataType
{
    Vedio=0,          //视频消息
    Audio=1,          //语音
    RequestConnect=2, //请求连接
    AcceptConnect=3,  //接收连接
    RefuseConnect=4,  //拒绝连接
    DisConnect=5,     //挂断连接

    Register=6,       //注册到服务器

};

typedef std::map<uint16_t,sockaddr_in> clientsMap;

class Server
{
public:
	Server(int port);
	~Server();
	void closeSocket();
	bool init();
	void run();
	
	static bool run_flag;
	
private:
	bool initSocket();
	void serveThread(const struct sockaddr_in& addr);
	void printThread();
	void msgTransmit(const uint8_t* buf, int len);
private:
	
	string socket_ip_;
	int socket_port_;
	int udp_fd_;

	struct sockaddr_in sockaddr_;
	clientsMap clients_;

};

bool Server::run_flag = true; 

Server::Server(int port)
{
	udp_fd_ = -1;
	socket_ip_ = "0.0.0.0";
	socket_port_ = port;
}

Server::~Server()
{
	
}

void Server::closeSocket()
{
	if(udp_fd_ != -1)
		close(udp_fd_);
}

bool Server::initSocket()
{
	bzero(&sockaddr_,sizeof(sockaddr_));//init 0

	sockaddr_.sin_port = htons(socket_port_);
	sockaddr_.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, socket_ip_.c_str(), &sockaddr_.sin_addr);
	if(convert_ret !=1)
	{
		perror("convert socket ip failed, please check the format!");
		return false;
	}
	
	//UDP
	udp_fd_ = socket(PF_INET,SOCK_DGRAM , 0);
	if(udp_fd_ < 0)
	{
		perror("build socket error");
		return false;
	}
	int udp_opt = 1;
	setsockopt(udp_fd_, SOL_SOCKET, SO_REUSEADDR, &udp_opt, sizeof(udp_opt));
	
	int ret = bind(udp_fd_, (struct sockaddr*)&sockaddr_,sizeof(sockaddr_));
	if(ret < 0)
	{
		std::cout << "udp bind ip: "<< socket_ip_ 
				  << ",port: "<< socket_port_ << "failed!!" << std:: endl;
		return false;
	}
	return true;
}


bool Server::init()
{
	if(!initSocket())
		return false;

	std::cout << "port: " << socket_port_ << "  init ok." << std:: endl;
	
	std::thread t1 = std::thread(&Server::printThread,this);
	t1.detach();
	
	return true;
}


void Server::printThread()
{
	int i=0;
	int interval = 5; 
	while(run_flag)
	{
		sleep(interval);
		printf("this server has been running for %d seconds.\n",interval*(++i));
	}
}

void Server::run()
{
	const int BufLen = 100000;
	uint8_t *recvbuf = new uint8_t [BufLen+1];

	struct sockaddr_in client_addr;
	socklen_t clientLen = sizeof(client_addr);
	
	while(run_flag)
	{
		int len = recvfrom(udp_fd_, recvbuf, BufLen,0,(struct sockaddr*)&client_addr, &clientLen);
		
		if(recvbuf[0] != 0x66 || recvbuf[1] != 0xcc)
			continue;
		uint8_t type = recvbuf[4];
		
		if(type == Register)
		{
			uint16_t clientId = recvbuf[7]*256 + recvbuf[6];
			//uint16_t clientPort = recvbuf[11]*256 + recvbuf[10];
			//client_addr.sin_port = clientPort;
			
			clients_[clientId] = client_addr;
			cout << "client id: " << clientId << " regist ok!" << endl;
			continue;
		}
		// 转发消息 
		msgTransmit(recvbuf,len);
	}
	
	delete [] recvbuf;
}

void Server::msgTransmit(const uint8_t* buf, int len)
{
	uint16_t dstClientId = buf[9]*256 + buf[8];
	auto it = clients_.find(dstClientId);
	if (it == clients_.end()) //未查找到目标客户端 
	{
		cout << "No client : " << dstClientId << endl;
		return;
	}
	int send_len = sendto(udp_fd_, buf, len, 0, (struct sockaddr*)&clients_[dstClientId], sizeof(sockaddr_in));
	cout << "transmitting : " << send_len << " bytes to id: " << dstClientId << "\tport：" << clients_[dstClientId].sin_port << endl;
}


void sigint_handler(int signal_num)
{
	//std::cout << "signal_num: " << signal_num << std::endl;
	Server::run_flag = false;
	usleep(100000);
	exit(0);
}

int main(int argc,char** argv)
{
	signal(SIGINT, sigint_handler);
	int port = 8617;
	if(argc > 1)
		port = atoi(argv[1]);
	Server server(port);
	if(!server.init())
		return 0;
	server.run();
	server.closeSocket();
	return 0;
}

// scp * root@aliyun:/root/seu/wendao/remote_driverless_server
