#include "remoteCmdHandler.h"

RemoteCmdHandler::RemoteCmdHandler()
{
	m_serverIp = "202.5.17.216";
	m_registerPort = 8617;
	m_myId = 5050;
}

bool RemoteCmdHandler::init()
{
	m_fd = initSocket(); //初始化socket 
	
	//配置服务器地址 (注册地址) 
	struct sockaddr_in serverAddr;
	bzero(&m_registerAddr,sizeof(m_registerAddr));
	m_registerAddr.sin_port = htons(m_registerPort); 
	m_registerAddr.sin_family = AF_INET;
	int convert_ret = inet_pton(AF_INET, m_serverIp.c_str(), &m_registerAddr.sin_addr);
	if(convert_ret !=1)
	{
		std::cout << "convert socket ip failed, please check the format!" << std::endl;
		return false;
	}
	
	//启动数据接收线程，在向服务器发送数据之前无法接收消息 
	std::thread t(&RemoteCmdHandler::receiveThread, this, m_fd);
	
	等待 
	
	registerToServer()
	
}

//数据接收线程 
//注册到服务器时，此线程用于接收服务器注册地址发送的回应注册信息
//当客户端收到服务器分配的新端口号后，开始接收新端口数据 
/* 
*@param fd socket套接字
*@param addr 服务器地址 
*/
void RemoteCmdHandler::receiveThread(const int fd)
{
	const int BufLen = 50;
	char *recvBuf = new char[BufLen];
	const transPack_t *package = (const transPack_t *)recvbuf;
	
	struct sockaddr_in addr;
	socklen_t socklen; 
	
	m_runFlag = true;
	while(m_runFlag)
	{
		int len = recvfrom(fd, recvbuf, BufLen,0,(struct sockaddr*)&addr, &socklen);
		if(len <=0 ) continue;
		if(package->head[0] != 0x66 || package->head[1] != 0xcc)continue;
		
		if(ResponseRegister == package->type) //服务器回应注册,包含新端口号
	    {
	        uint16_t serverPort =
	            recvBuf[sizeof(transPack_t)]+recvBuf[sizeof(transPack_t)+1]*256;
			addr.sin_port = htons(serverPort);
			this->m_msgAddr =  addr; //保存服务器消息传输地址 
	        confirmRegister(fd, m_msgAddr); 
	    }
	    else if(RegisterOK == package->type) //服务器回应注册成功
	    {
	    	m_isRegisterOk = true; 
	    	
	        //启动心跳线程
	        std::thread t(&RemoteCmdHandler::heartBeatThread,this);
	        t.detach();
	    }
	    else if(RegisterFail == package->type) //服务器回应注册失败
	    {
	        std::cout << "register failed!" << std::endl;
	    }
	    else if(RequestConnect == package->type) //请求连接
	    {
	    	uint16_t callerId = package->senderId;
        
            transPack_t pkg;
            memcpy(&pkg, recvbuf, sizeof(transPack_t)); //拷贝接收到的消息！
            pkg.senderId = g_myId;
            pkg.receiverId = callerId;
            pkg.length = 0;
            pkg.type = AcceptConnect; 
            sendto(fd, (char *)&pkg, sizeof(pkg),
					(struct sockaddr*)&addr, sizeof(addr));
		} 	
		else if(HeartBeat == package->type) //心跳包 
		{
			m_heartBeatMutex.lock();
            m_serverLastHeartBeatTime = time(0);
            m_heartBeatMutex.unlock();
		}
		else if(ControlCmd == package->type) //控制指令
		{
			
			
			//此处使用一个函数指针，向外发布消息的函数 
		} 
	}
	
	delete [] recvBuf;
}

//确认注册
//当收到服务器的注册回应时调用此函数
//此函数首先向serverPort再次发送注册信息，以与服务器的消息转发socket建立连接
void RemoteCmdHandler::confirmRegister(const int fd, struct sockaddr_in addr)
{
    transPack_t package(RequestRegister);
    package.senderId = m_myId;
    for(int i=0; i<3; ++i)
    {
    	sendto(fd, (char *)&package, sizeof(package),
				(struct sockaddr*)&addr, sizeof(addr));
		
		std::this_thread::sleep_for(std::chrono::milliseconds(200)); 
    }
}


int RemoteCmdHandler::initSocket(const int port, const std::string ip, int time_out)
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


bool RemoteCmdHandler::registerToServer(const int fd, struct sockaddr_in addr)
{
    transPack_t package(RequestRegister);
    package.senderId = m_myId;
    
    int cnt = 0;
    m_isRegisterOk = false;
    while(!m_isRegisterOk) //(循环登录)
    {
        std::cout << "send register msg..." << std::endl;
        
        sendto(fd, (char *)&package, sizeof(package),
				(struct sockaddr*)&addr, sizeof(addr));
		
		std::this_thread::sleep_for(std::chrono::milliseconds(200)); 

		if(++cnt > 50)
		{
			std::cout << "register time out...";
			return false;
		}
    }
    return true;
}


//向服务器定时发送心跳包，并判断上次接收到服务器心跳包是否超时 
void RemoteCmdHandler::heartBeatThread(const int fd, struct sockaddr_in addr)
{
    m_serverLastHeartBeatTime = time(NULL); //此处必须初始化
    transPack_t heartBeatPkg(HeartBeat);
    
    while(m_runFlag && m_isRegisterOk)
    {
        m_heartBeatMutex.lock();
        //qDebug() << m_serverLastHeartBeatTime << "\t" << m_serverLastHeartBeatTime-time(0) ;
        bool disconnect = time(0) - m_serverLastHeartBeatTime > m_heartBeatInterval + m_maxHeartBeatDelay;
        m_heartBeatMutex.unlock();
        if(disconnect)
        {
            m_isRegisterOk = false;
            qDebug() << "server is shutdown" ;
            return;
        }

        sendto(fd, (char *)&heartBeatPkg, sizeof(heartBeatPkg),
				(struct sockaddr*)&addr, sizeof(addr));
				
        std::this_thread::sleep_for(std::chrono::seconds(m_heartBeatInterval)); 
    }
}

