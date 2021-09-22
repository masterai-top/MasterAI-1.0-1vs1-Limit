/**
* \file path.h
* \brief 路径封装类
*
* 提供文件路径的操作功能。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __PATH_H__
#define __PATH_H__

#include "utildef.h"
#include <string>

namespace dtutil
{
    /**
    * \brief 路径封装类
    * \ingroup file_group
    */
    class UTIL_EXPORT Path
    {
    public:
        /**
        * \brief 是否目录
        * \param path 路径
        * \return 是返回true，否则返回false
        */
        static bool IsDirectory(std::string path);

        /**
        * \brief 生成目录
        * \param path 路径
        * \return 成功返回true，否则返回false
        */
        static bool MakeDir(std::string path);

        /**
        * \brief 修正路径
        * \param path 路径
        * 将 win32 下的路径符号"\"改为"/"，
        * 并在路径后加上"/"
        */
        static void FixPath(std::string &path);

        /**
        * \brief 获取文件路径
        * \param file 完整文件名
        * \return 去掉文件名的路径
        */
        static std::string GetPath(std::string file);

        /**
        * \brief 获取文件名
        * \param file 完整文件名
        * \return 去掉路径的文件名
        */
        static std::string GetFileName(std::string file);

        /**
        * \brief 解析文件名
        * \param filename 完整文件名
        * \param path 返回的文件路径
        * \param file 返回的文件名
        * \param ext 返回的文件名后缀
        * \return 成功返回true，否则返回false
        */
        static bool ParseFileName(std::string filename, std::string &path, std::string &file, std::string &ext);

        /**
        * \brief 是否存在文件
        * \param file 文件名
        * \return 存在返回true，否则返回false
        */
        static bool ExistFile(std::string file);

        /**
        * \brief 文件改名
        * \param oldfile 旧文件名
        * \param newfile 新文件名
        * \return 成功返回true，否则返回false
        */
        static bool RenameFile(std::string oldfile, std::string newfile);
    };
}

#endif // __PATH_H__