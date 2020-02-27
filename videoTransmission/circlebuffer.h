#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <vector>
#include <mutex>

/*
 * CircleBuffer 环形数据存储区模板类
 * 包含读写锁
 * 两种存储模式，当缓冲区满时丢弃新数据、丢弃新数据
 */

template<typename T>
class CircleBuffer
{
private:
   std::vector<T> m_array;
   std::vector<bool> m_datasStatus;
   size_t m_size;
   int m_mode;
   size_t m_readIndex;
   size_t m_writeIndex;
   std::mutex m_readWriteMutex;
public:
    enum
    {
        KEEP_OLD_DATA,
        OVERWRITE_OLD_DATA,
    };
    CircleBuffer()
    {
        m_readIndex = m_writeIndex = 0;
        m_size = 0;
        m_mode = OVERWRITE_OLD_DATA;
    }

    //设置缓冲区大小，不可动态调节，如有需求，可添加动态配置函数
    bool setSize(size_t size)
    {
        if(m_size != 0)
            return false;
        m_size = size;
        m_array.resize(m_size);
        m_datasStatus = std::vector<bool>(m_size,false);
        return true;
    }

    size_t getSize(){return m_size;}

    //设置缓冲区工作模式
    bool setMode(int mode){ m_mode = mode;}

    bool read(T& data)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_size == 0)
            return false;
        if(m_datasStatus[m_readIndex])
        {
            data = m_array[m_readIndex];
            m_datasStatus[m_readIndex] = false;
            m_readIndex = (m_readIndex+1)%m_size;
            return true;
        }
        return false;
    }

    //缓冲区满：缓冲区所有位置被写入数据且未被读出
    //缓冲区空：1.还未开始写入数据，2.写入的数据均被读出

    bool write(const T& data)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_size == 0)
            return false;
        if(m_readIndex == m_writeIndex) //读写指针在同一位置
        {
            //当前位置可读,写指针位置可读表明缓冲区满
            if(m_datasStatus[m_writeIndex])
            {
                if(m_mode == CircleBuffer::OVERWRITE_OLD_DATA) //覆盖旧数据，保持数据最新
                {
                    m_array[m_writeIndex] = data;
                    m_writeIndex = (m_writeIndex+1)%m_size;
                    m_readIndex = (m_readIndex+1)%m_size;
                    return true;
                }
                else //保留旧数据，停止写入并返回写入失败
                    return false;
            }
        }
        m_array[m_writeIndex] = data;
        m_datasStatus[m_writeIndex] = true;
        m_writeIndex = (m_writeIndex+1)%m_size;
        return true;
    }
};

#endif // CIRCLEBUFFER_H
