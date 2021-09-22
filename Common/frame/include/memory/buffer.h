/**
* \file buffer.h
* \brief 缓存数据类
*
* 初始化一段内存，往内存中放数据，
* 满了会自动增长。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "framedef.h"
#include "typedef.h"
#include <string>

namespace frame
{
    /**
    * \brief 缓存数据类
    * \ingroup memory_group
    * 当缓存数据分配的内存用完时，会自动增长
    */
    class FRAME_EXPORT Buffer
    {
    protected:
        /**
        * \brief 构造函数
        * 仅用于被继承
        */
        Buffer(void);

    public:
        /**
        * \brief 构造函数
        * \param size 初始大小
        * \param alloc 每次自动分配大小
        */
        Buffer(uint32 size, uint32 alloc = 1024);

        /**
        * \brief 析构函数
        */
        virtual ~Buffer(void);

    public:
        /**
        * \brief 写入数据
        * \param buffer 要写入的缓冲数据
        * \param len 写入的缓冲数据长度
        * \return 成功返回true，否则返回false
        */
        bool Write(const char *buffer, uint32 len);

        /**
        * \brief 读出指定长度的数据
        * \param buffer 读入的缓冲数据
        * \param len 读入的缓冲数据长度
        * \return 成功返回true，否则返回false
        */
        bool Read(void *buffer, uint32 len);

        /**
        * \brief 写入模板数据
        * \param data 模板类型数据
        * \return 成功返回true，否则返回false
        */
        template<class T>
        bool Push( T &data ) { return Write( (const char *)&data, sizeof(T) ); }

        /**
        * \brief 读出模板数据
        * \param data 模板类型数据
        * \return 成功返回true，否则返回false
        */
        template<class T>
        bool Pop( T &data ) { return Read( &data, sizeof(T) ); }

        /**
        * \brief 写入字符串
        * 模板特化，让适用于 string 数据长度是变长的
        * \param str 要写入的字符串
        * \return 成功返回true，否则返回false
        */
        bool Push( std::string &str );

        /**
        * \brief 写入字符串
        * 模板特化，让适用于 const string 数据长度是变长的
        * \param str 要写入的字符串
        * \return 成功返回true，否则返回false
        */
        bool Push( const std::string &str );

        /**
        * \brief 读取字符串
        * 模板特化，让适用于 string 数据长度是变长的
        * \param str 读取的字符串
        * \return 成功返回true，否则返回false
        */
        bool Pop( std::string &str );

        /**
        * \brief 写入字符串
        * 模板特化，让适用于 wstring 数据长度是变长的
        * \param wstr 要写入的字符串
        * \return 成功返回true，否则返回false
        */
        bool Push( std::wstring &wstr );

        /**
        * \brief 写入字符串
        * 模板特化，让适用于 const wstring 数据长度是变长的
        * \param wstr 要写入的字符串
        * \return 成功返回true，否则返回false
        */
        bool Push( const std::wstring &wstr );

        /**
        * \brief 读取字符串
        * 模板特化，让适用于 wstring 数据长度是变长的
        * \param wstr 读取的字符串
        * \return 成功返回true，否则返回false
        */
        bool Pop( std::wstring &wstr );

        /**
        * \brief 写入固定长度缓冲数据
        * \param buffer 要写入的缓冲数据
        * \param len 要写入的缓冲数据长度
        * \return 成功返回true，否则返回false
        */
        bool Push( const char *buffer, uint32 len );

        /**
        * \brief 读取固定长度缓冲数据
        * \param buffer 读取的缓冲数据
        * \param len 读取的缓冲数据长度
        * \return 成功返回true，否则返回false
        */
        bool Pop( char *buffer, uint32 &len );

        /**
        * \brief 获取当前缓冲指针所指的数据地址，适合直接操作缓冲区
        * \return 缓冲指针所指的数据地址
        */
        char * GetBuffer();

        /**
        * \brief 把当前指针向后移动 offset 个字节
        * \param offset 移动字节数
        * \return 移动前的指针位置
        * 设置 offset 即可获得当前缓冲指针偏移
        * 如果操作失败，Seek返回0xFFFFFFFF
        */
        uint32 Seek(uint32 offset);

        /**
        * \brief 把当前指针移动到指定位置
        * \param pos 要移动到的位置
        * \return 移动前的指针位置
        * 如果操作失败，SeekTo返回0xFFFFFFFF
        */
        uint32 SeekTo(uint32 pos = 0);

        /**
        * \brief 获取当前偏移位置
        * \return 当前偏移位置
        */
        uint32 GetOffset();

        /**
        * \brief 获取剩余长度
        * 读取数据时，获取的是剩余数据长度
        * 写入数据时，获取的是有效缓冲区大小
        * \return 剩余长度
        */
        uint32 GetLeftLen();

        /**
        * \brief 获取缓冲区大小
        * \return 缓冲区大小
        */
        uint32 GetSize();

        /**
        * \brief 克隆缓冲区数据
        * \param buffer 需克隆的对象
        * \return 成功返回true，否则返回false
        */
        bool Clone(Buffer &buffer);

    private:
        char    *m_pBuffer;     ///< Buffer指针
        uint32  m_nSize;        ///< Buffer长度
        uint32  m_nOffset;      ///< 偏移位置
        uint32  m_nAlloc;       ///< 内存不足时每次分配大小
    };
}

#endif // __BUFFER_H__