#ifndef FIFO_HPP
#define FIFO_HPP

#include <fcntl.h>    // O_WRONLY
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>

//用于进程通讯，先进先出
class Fifo
{
public:
    Fifo()  {m_fd = -1;}
    ~Fifo() {close();  }

#ifndef _WIN32
    bool open(const std::string& name,const std::string& mode)
    {
        if((m_fd = mkfifo(name.c_str(), 0666)) < 0)// 创建FIFO管道,所有人可读可写
        {
            if(errno!=EEXIST) //若不是已存在导致的创建失败
            {
                std::cerr << "Create FIFO Failed" << std::endl;
                return false;
            }
        }

        //O_RDWR   读写
        //O_WRONLY 只写
        //O_RDONLY 只读

        if(mode=="w")//只写
            m_mode = O_WRONLY;
        else if(mode=="r")//只读
            m_mode = O_RDONLY;
        else if(mode == "wr")//读写
            m_mode = O_RDWR;
        else
        {
            std::cerr << "Open FIFO Mode Error! [w, r, wr]." << std::endl;
            return false;
        }

        //均以读写方式打开，在发送和接收函数中进行读写限定，防止无接收方时写阻塞
        if((m_fd= ::open(name.c_str(), O_RDWR)) < 0)
        {
            std::cerr << "Open FIFO " << name << " Failed" << std::endl;
                return false;
        }

        return true;
    }

    int send(const void* buf, int len)
    {
        if(m_mode == O_RDONLY)
        {
            std::cerr << "Current Mode is Only Read!, Must Not call send!" << std::endl;
            return -1;
        }
        return write(m_fd, buf, len);
    }

    int receive(void *buf, int max_len)
    {
        if(m_mode == O_WRONLY)
        {
            std::cerr << "Current Mode is Only Write!, Must Not call receive!" << std::endl;
            return -1;
        }
        return read(m_fd, buf, max_len);
    }
#endif

    void close()
    {
        if(m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    int getProcessId()
    {
        return getpid();
    }

private:
    std::string m_fifoName;
    int m_mode;
    int m_fd;
};



#endif // FIFO_HPP
