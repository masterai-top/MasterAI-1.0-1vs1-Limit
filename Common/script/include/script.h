/**
* \file script.h
* \brief 资源脚本引擎类头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "typedef.h"
#include "scriptdef.h"

namespace dtscript
{
    class ICsvIterator;
    class IIniIterator;
    class IXmlIterator;

    /**
    * \brief 脚本处理回调类
    * \ingroup script_group
    */
    class IScriptHandler
    {
    public:
        virtual ~IScriptHandler() = 0;
        
        /**
        * \brief csv文件加载回调
        * \param szFileName 文件名
        * \param pFilePoint 文件数据指针
        * \return 成功处理回调返回true，否则返回false
        */
        virtual bool OnFileLoad(const char *szFileName, ICsvIterator *pFilePoint) { return false; }

        /**
        * \brief ini文件加载回调
        * \param szFileName 文件名
        * \param pFilePoint 文件数据指针
        * \return 成功处理回调返回true，否则返回false
        */
        virtual bool OnFileLoad(const char *szFileName, IIniIterator *pFilePoint) { return false; }

        /**
        * \brief xml文件加载回调
        * \param szFileName 文件名
        * \param pFilePoint 文件数据指针
        * \return 成功处理回调返回true，否则返回false
        */
        virtual bool OnFileLoad(const char *szFileName, IXmlIterator *pFilePoint) { return false; }
    };

    /**
    * \brief 资源脚本引擎类
    * \ingroup script_group
    */
    class SCRIPT_EXPORT ScriptEngine
    {
    public:
        /**
        * \brief 加载资源文件
        * \param szFileName 资源文件
        * \param pScriptHandler 文件加载后的回调指针，实现该方法将文件数据存起来
        * \return 加载资源文件成功返回true，否则返回false
        *
        * 该函数加载资源后，会回调到传入的回调接口，
        * 回调时获取的文件数据指针可以读取出加载的文件数据，
        * 不需要释放，在调用完回调后内部自己删除了。
        * 所以在回调处理中，应用程序需处理自己的数据存储。
        */
        static bool LoadFile(const char *szFileName, IScriptHandler *pScriptHandler);
    };
}

#endif // __SCRIPT_H__