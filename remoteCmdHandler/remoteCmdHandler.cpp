#include "remoteCmdHandler.h"

RemoteCmdHandler::RemoteCmdHandler():
	m_heartBeatInterval(5),
    m_maxHeartBeatDelay(3),
    m_cmd_callback(NULL),
	m_registerTimeOut(15) 
{
	m_serverIp = "202.5.17.216";
	m_registerPort = 8617;
	m_myId = 5050;
	
}
RemoteCmdHandler::~RemoteCmdHandler()
{
	this->stop();
}

//����Զ�̿��ƴ����� 
bool RemoteCmdHandler::start()
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
	t.detach();
	
	bool ok = this->registerToServer(m_fd,m_registerAddr);
	return ok;
}

void RemoteCmdHandler::stop()
{
	m_runFlag = false;
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
	const PkgHeader *header = (const PkgHeader *)recvBuf;
	
	struct sockaddr_in addr;
	socklen_t socklen; 
	
	m_runFlag = true;
	while(m_runFlag)
	{
		int len = recvfrom(fd, recvBuf, BufLen,0,(struct sockaddr*)&addr, &socklen);
		if(len <=0 ) continue;
		if(header->head[0] != 0x66 || header->head[1] != 0xcc)continue;
		
		//std::cout << "received msg, type: " << int(header->type) << std::endl;
		
		if(PkgType_ResponseRegister == header->type) //��������Ӧע��,�����¶˿ں�
	    {
	        uint16_t serverPort =
	            recvBuf[sizeof(PkgHeader)]+recvBuf[sizeof(PkgHeader)+1]*256;
			addr.sin_port = htons(serverPort);
			this->m_msgAddr =  addr; //�����������Ϣ�����ַ 
	        confirmRegister(fd, m_msgAddr); 
	    }
	    else if(PkgType_RegisterOK == header->type) //��������Ӧע��ɹ�
	    {
	    	m_isRegisterOk = true; 
	    	
	        //���������߳�
	        std::thread t(&RemoteCmdHandler::heartBeatThread,this, fd, m_msgAddr); 
	        t.detach();
	    }
	    else if(PkgType_RegisterFail == header->type) //��������Ӧע��ʧ��
	    {
	        std::cout << "register failed!" << std::endl;
	    }
	    else if(PkgType_RequestConnect == header->type) //��������
	    {
	    	uint16_t callerId = header->senderId;
            pkgHeader_t pkg;
            memcpy(&pkg, recvBuf, sizeof(pkgHeader_t)); //�������յ�����Ϣ��
            pkg.senderId = this->m_myId;
            pkg.receiverId = callerId;
            pkg.length = 0;
            pkg.type = PkgType_AcceptConnect; 
            //sendto(fd, (char *)&pkg, 0,  sizeof(pkg), (struct sockaddr*)&addr, sizeof(addr));
            int len = sendto(fd, (char *)&pkg, sizeof(pkgHeader_t),0 , (struct sockaddr*)&addr, sizeof(addr));
		}
		else if(PkgType_HeartBeat == header->type) //������ 
		{
			m_heartBeatMutex.lock();
            m_serverLastHeartBeatTime = time(0);
            m_heartBeatMutex.unlock();
		}
		else if(PkgType_ControlCmd == header->type) //����ָ��
		{
			if(m_cmd_callback == NULL)
				std::cout << "please bind callback function!" << std::endl;
			else	
			{
				controlCmdPkg_t *cmdPkg = (controlCmdPkg_t *)recvBuf;
				m_cmd_callback(cmdPkg->cmd);
			}
		} 
		
	}
	
	delete [] recvBuf;
	recvBuf = NULL;
}

//ȷ��ע��
//���յ���������ע���Ӧʱ���ô˺���
//�˺���������serverPort�ٴη���ע����Ϣ���������������Ϣת��socket��������
void RemoteCmdHandler::confirmRegister(const int fd, struct sockaddr_in addr)
{
    PkgHeader package(PkgType_RequestRegister);
    package.senderId = m_myId;
    for(int i=0; i<3; ++i)
    {
    	int len = sendto(fd, (char *)&package, sizeof(package), 0, 
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
    PkgHeader package(PkgType_RequestRegister);
    package.senderId = m_myId;
    
    int cnt = 0;
    m_isRegisterOk = false;
    float repeatInterval = 0.5; //s
    
    std::cout << "registering to server..." << std::endl;
    
    while(!m_isRegisterOk) //(ѭ����¼)
    {
        
        
        sendto(fd, (char *)&package, sizeof(package), 0, 
				(struct sockaddr*)&addr, sizeof(addr));
		
		std::this_thread::sleep_for(std::chrono::milliseconds(int(repeatInterval*1000))); 

		if(++cnt > m_registerTimeOut/repeatInterval)
		{
			std::cout << "register time out...";
			return false;
		}
    }
    std::cout << "Successfully registered to server." << std::endl;
    return true;
}

//���������ʱ���������������ж��ϴν��յ��������������Ƿ�ʱ 
void RemoteCmdHandler::heartBeatThread(const int fd, struct sockaddr_in addr)
{
    m_serverLastHeartBeatTime = time(NULL); //�˴������ʼ��
    PkgHeader heartBeatPkg(PkgType_HeartBeat);
    heartBeatPkg.senderId = m_myId;
    
    while(m_runFlag && m_isRegisterOk)
    {
        m_heartBeatMutex.lock();
        bool disconnect = time(0) - m_serverLastHeartBeatTime > m_heartBeatInterval + m_maxHeartBeatDelay;
        m_heartBeatMutex.unlock();
        if(disconnect)
        {
            m_isRegisterOk = false;
            std::cout << "server is shutdown" << std::endl;
            return;
        }

        sendto(fd, (char *)&heartBeatPkg, sizeof(heartBeatPkg),0,
				(struct sockaddr*)&addr, sizeof(addr));
				
        std::this_thread::sleep_for(std::chrono::seconds(m_heartBeatInterval)); 
    }
}

