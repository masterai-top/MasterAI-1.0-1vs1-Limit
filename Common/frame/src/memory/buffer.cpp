/**
* \file buffer.cpp
* \brief 缓存数据类函数的实现
*/

#include "pch.h"
#include "memory/buffer.h"


namespace frame
{
    /**
    * \brief 构造函数
    * 仅用于被继承
    */
    Buffer::Buffer(void)
    {
        m_pBuffer = NULL;
        m_nSize = 0;
        m_nOffset = 0;
        m_nAlloc = 0;
    }

    /**
    * \brief 构造函数
    * \param size 初始大小
    * \param alloc 每次自动分配大小
    */
    Buffer::Buffer(uint32 size, uint32 alloc /*= 1024*/)
    {
        m_nSize = size;
        m_nAlloc = alloc > 0 ? alloc : 1024;
        m_nOffset = 0;
        m_pBuffer = NULL;
        if (size > 0)
        {
            m_pBuffer = new char[size];
        }
    }

    /**
    * \brief 析构函数
    */
    Buffer::~Buffer(void)
    {
        if (m_pBuffer)
        {
            delete[] m_pBuffer;
            m_pBuffer = NULL;
        }

        m_nSize = 0;
        m_nAlloc = 0;
        m_nOffset = 0;
    }

    /**
    * \brief 写入数据
    * \param buffer 要写入的缓冲数据
    * \param len 写入的缓冲数据长度
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Write(const char* buffer, uint32 len)
    {
        if (NULL == buffer || 0 == len)
        {
            return false;
        }

        // 缓冲区不够，自动扩展
        if (m_nSize - m_nOffset < len)
        {
            uint32 nNeed = len + m_nSize - m_nOffset;       // 还差长度
            if (m_nAlloc == 0)
            {
                m_nAlloc = 1024;
            }
            uint32 nAlloc = (nNeed + m_nAlloc) / m_nAlloc * m_nAlloc;   // 需新增内存大小
            m_nSize += nAlloc;      // 新buffer总长度

            // 分配新内存
            char* pOldBuffer = m_pBuffer;
            m_pBuffer = new char[m_nSize];
            if (NULL == m_pBuffer)
            {
                return false;
            }

            // 将原数据拷贝过来
            memcpy(m_pBuffer, pOldBuffer, m_nOffset);
            // 删除原内存
            delete[] pOldBuffer;
        }

        memcpy((char*)m_pBuffer+m_nOffset, buffer, len);
        m_nOffset += len;

        return true;
    }

    /**
    * \brief 读出指定长度的数据
    * \param buffer 读入的缓冲数据
    * \param len 读入的缓冲数据长度
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Read(void* buffer, uint32 len)
    {
        if (NULL == buffer || 0 == len)
        {
            return false;
        }

        if (m_nOffset + len > m_nSize)
        {
            return false;
        }

        memcpy(buffer, m_pBuffer+m_nOffset, len);
        m_nOffset += len;

        return true;
    }

    /**
    * \brief 写入字符串
    * 模板特化，让适用于 string 数据长度是变长的
    * \param str 要写入的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Push( std::string& str )
    {
        uint32 size = (uint32)(str.length());
        Push(size);
        if (!str.empty())
        {
            return Write(str.data(), size);
        }

        return true;
    }

    /**
    * \brief 写入字符串
    * 模板特化，让适用于 const string 数据长度是变长的
    * \param str 要写入的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Push( const std::string& str )
    {
        uint32 size = (uint32)(str.length());
        Push(size);
        if (!str.empty())
        {
            return Write(str.data(), size);
        }

        return true;
    }

    /**
    * \brief 读取字符串
    * 模板特化，让适用于 string 数据长度是变长的
    * \param str 读取的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Pop( std::string& str )
    {
        uint32 len = 0;
        Pop(len);
        if (len > 0)
        {
            uint32 size = len + sizeof(char);
            char* buffer = new char[size];
            if (!Read(buffer, len))
            {
                delete[] buffer;
                return false;
            }
            buffer[len] = 0;
            str = buffer;
            delete[] buffer;
        }

        return true;
    }

    /**
    * \brief 写入字符串
    * 模板特化，让适用于 wstring 数据长度是变长的
    * \param wstr 要写入的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Push( std::wstring& wstr )
    {
        uint32 size = (uint32)(wstr.length() * sizeof(wchar_t));
        Push(size);
        if (!wstr.empty())
        {
            return Write((const char*)wstr.data(), size);
        }

        return true;
    }

    /**
    * \brief 写入字符串
    * 模板特化，让适用于 const wstring 数据长度是变长的
    * \param wstr 要写入的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Push( const std::wstring& wstr )
    {
        uint32 size = (uint32)(wstr.length() * sizeof(wchar_t));
        Push(size);
        if (!wstr.empty())
        {
            return Write((const char*)wstr.data(), size);
        }

        return true;
    }
    
    /**
    * \brief 读取字符串
    * 模板特化，让适用于 wstring 数据长度是变长的
    * \param wstr 读取的字符串
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Pop( std::wstring& wstr )
    {
        uint32 size = 0;
        Pop(size);
        uint32 len = size / sizeof(wchar_t);
        if (len > 0)
        {
            wchar_t* buffer = new wchar_t[len+1];
            if (!Read(buffer, size))
            {
                delete[] buffer;
                return false;
            }
            buffer[len] = 0;
            wstr = buffer;
            delete[] buffer;
        }

        return true;
    }

    /**
    * \brief 写入固定长度缓冲数据
    * \param buffer 要写入的缓冲数据
    * \param len 要写入的缓冲数据长度
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Push( const char* buffer, uint32 len )
    {
        Push(len);

        if (0 != len)
        {
            return Write(buffer, len);
        }

        return true;
    }

    /**
    * \brief 读取固定长度缓冲数据
    * \param buffer 读取的缓冲数据
    * \param len 读取的缓冲数据长度
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Pop( char* buffer, uint32& len )
    {
        Pop(len);

        if (0 != len)
        {
            return Read(buffer, len);
        }

        return true;
    }

    /**
    * \brief 获取当前缓冲指针所指的数据地址，适合直接操作缓冲区
    * \return 缓冲指针所指的数据地址
    */
    char* Buffer::GetBuffer()
    {
        return m_pBuffer;
    }

    /**
    * \brief 把当前指针向后移动 offset 个字节
    * \param offset 移动字节数
    * \return 移动前的指针位置
    * 设置 offset 即可获得当前缓冲指针偏移
    * 如果操作失败，Seek返回0xFFFFFFFF
    */
    uint32 Buffer::Seek(uint32 offset)
    {
        if (m_nOffset + offset > m_nSize)
        {
            return 0xFFFFFFFF;
        }

        uint32 nOldPos = m_nOffset;
        m_nOffset += offset;

        return nOldPos;
    }

    /**
    * \brief 把当前指针移动到指定位置
    * \param pos 要移动到的位置
    * \return 移动前的指针位置
    * 如果操作失败，SeekTo返回0xFFFFFFFF
    */
    uint32 Buffer::SeekTo(uint32 pos /*= 0*/)
    {
        if (pos > m_nSize)
        {
            return 0xFFFFFFFF;
        }

        uint32 nOldPos = m_nOffset;
        m_nOffset = pos;

        return nOldPos;
    }

    /**
    * \brief 获取当前偏移位置
    * \return 当前偏移位置
    */
    uint32 Buffer::GetOffset()
    {
        return m_nOffset;
    }

    /**
    * \brief 获取剩余长度
    * 读取数据时，获取的是剩余数据长度
    * 写入数据时，获取的是有效缓冲区大小
    * \return 剩余长度
    */
    uint32 Buffer::GetLeftLen()
    {
        return m_nSize - m_nOffset;
    }

    /**
    * \brief 获取缓冲区大小
    * \return 缓冲区大小
    */
    uint32 Buffer::GetSize()
    {
        return m_nSize;
    }

    /**
    * \brief 克隆缓冲区数据
    * \param buffer 需克隆的对象
    * \return 成功返回true，否则返回false
    */
    bool Buffer::Clone(Buffer& buffer)
    {
        if (m_nSize > buffer.m_nSize)
        {
            // 修改原内存数据
            memset(m_pBuffer, 0, m_nSize);
            memcpy(m_pBuffer, buffer.m_pBuffer, buffer.m_nSize);
            m_nOffset = buffer.m_nOffset;
        }
        else
        {
            // 分配新内存
            char* pOldBuffer = m_pBuffer;
            m_nSize = buffer.m_nSize;
            m_pBuffer = new char[m_nSize];
            memcpy(m_pBuffer, buffer.m_pBuffer, buffer.m_nSize);
            m_nOffset = buffer.m_nOffset;
            delete[] pOldBuffer;
        }

        return true;
    }
}
