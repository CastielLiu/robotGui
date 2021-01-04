#ifndef CIRCLEBUFFER_H
#define CIRCLEBUFFER_H

#include <vector>
#include <mutex>
#include <iostream>

/*
 * CircleBuffer 环形数据存储区模板类
 * 内置读写锁，多线程访问时无需手动加锁
 * 两种存储模式：当缓冲区满时丢弃新数据、丢弃旧数据
 */
template<typename T>
class CircleBuffer
{
private:
   std::vector<T> m_array; //存储数据
   size_t m_size;          //环形区中数据个数
   size_t m_capacity;      //环形区容量
   int m_mode;             //环形区模式，覆盖旧数据or丢弃新数据
   size_t m_readIndex;     //读指针
   size_t m_writeIndex;    //写指针
   std::mutex m_readWriteMutex; //读写锁
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

    //设置缓冲区大小
    bool reserve(size_t n)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        m_array.clear();
        m_array.resize(n);
        m_capacity = n;
        m_readIndex = m_writeIndex = 0; //读写指针复位
        m_size = 0;
        return true;
    }

    size_t size()
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        return m_size;
    }

    size_t capacity()
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        return m_capacity;
    }

    //设置缓冲区工作模式
    void setMode(int mode){ m_mode = mode;}

    //读取一个数据并更新读指针
    bool read(T& data)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_size == 0)
            return false;
        data = m_array[m_readIndex];
        m_readIndex = (m_readIndex+1)%m_capacity;
        --m_size; //更新数据个数

        return true;
    }

    //只读，不更新读指针
    bool onlyRead(T& data)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_size == 0)
            return false;
        data = m_array[m_readIndex];
        return true;
    }

    //弹出顶端数据
    bool pop_begin()
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_size == 0)
            return false;
        m_readIndex = (m_readIndex+1)%m_capacity;
        --m_size; //更新数据个数
    }

    //缓冲区满：缓冲区所有位置被写入数据且未被读出
    //缓冲区空：1.还未开始写入数据，2.写入的数据均被读出
    bool write(const T& data)
    {
        std::lock_guard<std::mutex> lck(m_readWriteMutex);
        if(m_capacity == 0)
            return false;

        //读写指针在同一位置 1. 数据为空 2. 数据已满
        if(m_readIndex == m_writeIndex)
        {
            if(m_size == 0) //环形区中无可用数据
            {
                m_array[m_writeIndex] = data;
                m_writeIndex = (m_writeIndex+1)%m_capacity;
                ++m_size; //更新数据个数
            }
            else if(m_mode == CircleBuffer::OVERWRITE_OLD_DATA)
            {
                //覆盖旧数据，保持数据最新,读写指针同时后移
                m_array[m_writeIndex] = data;
                m_writeIndex = (m_writeIndex+1)%m_capacity;
                m_readIndex = (m_readIndex+1)%m_capacity;
            }
            else
            {
                //保留旧数据，停止写入并返回写入失败
                return false;
            }
        }
        else
        {
            m_array[m_writeIndex] = data;
            m_writeIndex = (m_writeIndex+1)%m_capacity;
            ++m_size; //更新数据个数
        }

        return true;
    }
};

#endif // CIRCLEBUFFER_H
