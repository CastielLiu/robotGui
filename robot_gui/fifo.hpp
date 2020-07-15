#ifndef FIFO_HPP
#define FIFO_HPP

#include <fcntl.h>    // O_WRONLY
#include <cerrno>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
#include <cstdio>

//进程通讯，先进先出
class Fifo
{
public:
    Fifo()
    {
        m_fd = -1;
    }
    ~Fifo()
    {
        close();
    }

    bool open(const std::string& name,const std::string& mode)
    {
        if((m_fd=mkfifo(name.c_str(), 0666)) < 0)// 创建FIFO管道
        {
            if(errno!=EEXIST) //若不是已存在导致的创建失败
            {
                std::cerr << "Create FIFO Failed" << std::endl;
                return false;
            }
        }

        if((mode=="w") && (m_fd= ::open(name.c_str(), O_WRONLY)) < 0) //只写
        {
            std::cerr << "Open FIFO " << name << " Failed" << std::endl;
                return false;
        }
        else if((mode=="r") && (m_fd = ::open(name.c_str(), O_RDONLY)) < 0)//只读
        {
            std::cerr << "Open FIFO " << name << " Failed" << std::endl;
                return false;
        }
        else if((mode == "wr") && (m_fd = ::open(name.c_str(), O_RDWR)) < 0) //读写
        {
            std::cerr << "Open FIFO " << name << " Failed" << std::endl;
                return false;
        }
        else if(m_fd < 0)
        {
            std::cerr << "Open FIFO Mode Error! [w, r, wr]." << std::endl;
            return false;
        }
        return true;
    }

    void close()
    {
        if(m_fd >= 0)
        {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    int send(const void* buf, int len)
    {
        return write(m_fd, buf, len);
    }

    int receive(void *buf, int max_len)
    {
        return read(m_fd, buf, max_len);
    }

    int getProcessId()
    {
        return getpid();
    }

private:
    std::string m_fifoName;
    int m_fd;

};



#endif // FIFO_HPP
