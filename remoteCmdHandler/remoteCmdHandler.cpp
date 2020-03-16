#include "remoteCmdHandler.h"

RemoteCmdHandler::RemoteCmdHandler()
{
	m_serverIp = "202.5.17.216";
	m_registerPort = 8617;
	m_myId = 5050;
}

bool RemoteCmdHandler::init()
{
	m_fd = initSocket(); //��ʼ��socket 
	
	//���÷�������ַ (ע���ַ) 
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
	
	//�������ݽ����̣߳������������������֮ǰ�޷�������Ϣ 
	std::thread t(&RemoteCmdHandler::receiveThread, this, m_fd);
	
	�ȴ� 
	
	registerToServer()
	
}

//���ݽ����߳� 
//ע�ᵽ������ʱ�����߳����ڽ��շ�����ע���ַ���͵Ļ�Ӧע����Ϣ
//���ͻ����յ�������������¶˿ںź󣬿�ʼ�����¶˿����� 
/* 
*@param fd socket�׽���
*@param addr ��������ַ 
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
		
		if(ResponseRegister == package->type) //��������Ӧע��,�����¶˿ں�
	    {
	        uint16_t serverPort =
	            recvBuf[sizeof(transPack_t)]+recvBuf[sizeof(transPack_t)+1]*256;
			addr.sin_port = htons(serverPort);
			this->m_msgAddr =  addr; //�����������Ϣ�����ַ 
	        confirmRegister(fd, m_msgAddr); 
	    }
	    else if(RegisterOK == package->type) //��������Ӧע��ɹ�
	    {
	    	m_isRegisterOk = true; 
	    	
	        //���������߳�
	        std::thread t(&RemoteCmdHandler::heartBeatThread,this);
	        t.detach();
	    }
	    else if(RegisterFail == package->type) //��������Ӧע��ʧ��
	    {
	        std::cout << "register failed!" << std::endl;
	    }
	    else if(RequestConnect == package->type) //��������
	    {
	    	uint16_t callerId = package->senderId;
        
            transPack_t pkg;
            memcpy(&pkg, recvbuf, sizeof(transPack_t)); //�������յ�����Ϣ��
            pkg.senderId = g_myId;
            pkg.receiverId = callerId;
            pkg.length = 0;
            pkg.type = AcceptConnect; 
            sendto(fd, (char *)&pkg, sizeof(pkg),
					(struct sockaddr*)&addr, sizeof(addr));
		} 	
		else if(HeartBeat == package->type) //������ 
		{
			m_heartBeatMutex.lock();
            m_serverLastHeartBeatTime = time(0);
            m_heartBeatMutex.unlock();
		}
		else if(ControlCmd == package->type) //����ָ��
		{
			
			
			//�˴�ʹ��һ������ָ�룬���ⷢ����Ϣ�ĺ��� 
		} 
	}
	
	delete [] recvBuf;
}

//ȷ��ע��
//���յ���������ע���Ӧʱ���ô˺���
//�˺���������serverPort�ٴη���ע����Ϣ���������������Ϣת��socket��������
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
	// ���õ�ַ�ɸ��� 
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &udp_opt, sizeof(udp_opt));
	if(time_out)
	{
		struct timeval timeout;
	    timeout.tv_sec = time_out;//��
	    //timeout.tv_usec = 0;//΢��
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
    while(!m_isRegisterOk) //(ѭ����¼)
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


//���������ʱ���������������ж��ϴν��յ��������������Ƿ�ʱ 
void RemoteCmdHandler::heartBeatThread(const int fd, struct sockaddr_in addr)
{
    m_serverLastHeartBeatTime = time(NULL); //�˴������ʼ��
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

