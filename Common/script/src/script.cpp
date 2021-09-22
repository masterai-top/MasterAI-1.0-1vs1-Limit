/**
* \file script.cpp
* \brief 资源脚本引擎类函数实现
*/

#include "pch.h"
#include "script.h"
#include "csv_adapter.h"
#include "ini_adapter.h"
#include "xml_adapter.h"
#include "strutil.h"
using namespace dtutil;


namespace dtscript
{

    IScriptHandler::~IScriptHandler() 
    {
    }
    
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
    bool ScriptEngine::LoadFile(const char *szFileName, IScriptHandler *pScriptHandler)
    {
        // 解析文件名及后缀名
        std::vector<std::string> vecFiles;
        size_t nCount = StrUtil::strtostrarr(szFileName, vecFiles, '.');
        if (nCount < 2)
        {
            return false;
        }

        // 得到文件后缀
        std::string strExt = vecFiles[nCount - 1];

        bool bSuccess = false;

        // csv 文件
        if (StrUtil::strncmp(strExt.c_str(), "csv", strlen("csv")) == 0)
        {
            CsvAdapter *pAdapter = new CsvAdapter();
            if (NULL == pAdapter)
            {
                return false;
            }

            do 
            {
                // 装载脚本
                if (!pAdapter->LoadScript(szFileName, false))
                {
                    break;
                }

                // 回调上层处理
                if (!pScriptHandler->OnFileLoad(szFileName, pAdapter))
                {
                    break;
                }

                bSuccess = true;

            } while(0);

            delete pAdapter;
        }
        // ini 文件
        else if (StrUtil::strncmp(strExt.c_str(), "ini", strlen("ini")) == 0)
        {
            IniAdapter *pAdapter = new IniAdapter();
            if (NULL == pAdapter)
            {
                return false;
            }

            do 
            {
                // 装载脚本
                if (!pAdapter->LoadScript(szFileName, false))
                {
                    break;
                }

                // 回调上层处理
                if (!pScriptHandler->OnFileLoad(szFileName, pAdapter))
                {
                    break;
                }

                bSuccess = true;

            } while(0);

            delete pAdapter;
        }
        // xml 文件
        else if (StrUtil::strncmp(strExt.c_str(), "xml", strlen("xml")) == 0)
        {
            XmlAdapter *pAdapter = new XmlAdapter();
            if (NULL == pAdapter)
            {
                return false;
            }

            do 
            {
                // 装载脚本
                if (!pAdapter->LoadScript(szFileName, false))
                {
                    break;
                }

                // 回调上层处理
                if (!pScriptHandler->OnFileLoad(szFileName, pAdapter))
                {
                    break;
                }

                bSuccess = true;

            } while(0);

            delete pAdapter;
        }
        // 未知类型文件
        else
        {
            return false;
        }

        return bSuccess;
    }
}

