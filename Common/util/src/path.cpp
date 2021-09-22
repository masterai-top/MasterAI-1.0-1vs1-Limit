/**
* \file path.cpp
* \brief 路径类函数的实现
*/

#include "pch.h"
#include "path.h"

#ifdef WIN32
#   include <shlwapi.h>
#   pragma comment(lib, "shlwapi")
#else
#   include <dirent.h>
#endif

namespace dtutil
{
    /**
    * \brief 是否目录
    * \param path 路径
    * \return 是返回true，否则返回false
    */
    bool Path::IsDirectory(std::string path)
    {
#ifdef WIN32
        return PathIsDirectoryA(path.c_str()) ? true : false;
#else
        DIR* dir = opendir(path.c_str());
        if (NULL != dir)
        {
            closedir(dir);
            dir = NULL;
            return true;
        }

        return false;
#endif
    }

    /**
    * \brief 生成目录
    * \param path 路径
    * \return 成功返回true，否则返回false
    */
    bool Path::MakeDir(std::string path)
    {
        if (0 == path.length())
        {
            return true;
        }

        std::string sub;
        FixPath(path);

        std::string::size_type pos = path.find('/');
        while (pos != std::string::npos)
        {
            std::string cur = path.substr(0, pos);
            if (cur.length() > 0 && !IsDirectory(cur))
            {
                bool result = false;
#ifdef WIN32
                result = CreateDirectoryA(cur.c_str(), NULL) ? true : false;
#else
                result = (mkdir(cur.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) == 0);
#endif
                if (!result)
                {
                    return false;
                }
            }

            pos = path.find('/', pos+1);
        }

        return true;
    }

    /**
    * \brief 修正路径
    * \param path 路径
    * 将 win32 下的路径符号"\"改为"/"，
    * 并在路径后加上"/"
    */
    void Path::FixPath(std::string &path)
    {
        if (path.empty())
        {
            return;
        }

        for(std::string::iterator it = path.begin(); it != path.end(); ++it)
        {
            if (*it == '\\')
            {
                *it = '/';
            }
        }

        if (path.at(path.length()-1) != '/')
        {
            path.append("/");
        }
    }

    /**
    * \brief 获取文件路径
    * \param file 完整文件名
    * \return 去掉文件名的路径
    */
    std::string Path::GetPath(std::string file)
    {
        std::string path = "";
        if (file.empty())
        {
            return path;
        }

        // 无斜杠，返回空
        std::string::size_type pos = file.rfind('\\');
        if (pos == std::string::npos)
        {
            pos = file.rfind('/');
            if (pos == std::string::npos)
            {
                return path;
            }
        }

        path = file.substr(0, pos);

        return path;
    }

    /**
    * \brief 获取文件名
    * \param file 完整文件名
    * \return 去掉路径的文件名
    */
    std::string Path::GetFileName(std::string file)
    {
        std::string filename = file;
        if (file.empty())
        {
            return filename;
        }

        // 无斜杠，返回空
        std::string::size_type pos = file.rfind('\\');
        if (pos == std::string::npos)
        {
            pos = file.rfind('/');
            if (pos == std::string::npos)
            {
                return filename;
            }
        }

        filename = file.substr(pos+1, file.length()-pos-1);

        return filename;
    }

    /**
    * \brief 解析文件名
    * \param filename 完整文件名
    * \param path 返回的文件路径
    * \param file 返回的文件名
    * \param ext 返回的文件名后缀
    * \return 成功返回true，否则返回false
    */
    bool Path::ParseFileName(std::string filename, std::string &path, std::string &file, std::string &ext)
    {
        if (filename.empty())
        {
            return false;
        }

        path.clear();
        file.clear();
        ext.clear();

        // 点的位置
        std::string::size_type point_pos = filename.rfind('.');

        // 斜杠的位置
        std::string::size_type pos = filename.rfind('\\');
        if (pos == std::string::npos)
        {
            pos = filename.rfind('/');
        }

        // 无点无斜杠
        if (pos == std::string::npos && point_pos == std::string::npos)
        {
            return false;
        }

        // 无点，当作路径
        if (point_pos == std::string::npos)
        {
            path = filename;
        }
        else
        {
            // 以斜杠结尾
            if (pos == filename.length() - 1)
            {
                path = filename.substr(0, pos);
            }
            // 非斜杠结尾
            else
            {
                if (pos != std::string::npos)
                {
                    path = filename.substr(0, pos);
                }
                file = filename.substr(pos+1, point_pos-pos-1);
                ext = filename.substr(point_pos+1, filename.length()-point_pos-1);
            }
        }

        return true;
    }

    /**
    * \brief 是否存在文件
    * \param file 文件名
    * \return 存在返回true，否则返回false
    */
    bool Path::ExistFile(std::string file)
    {
        if (-1 == access(file.c_str(), 0))
        {
            return false;
        }

        return true;
    }

    /**
    * \brief 文件改名
    * \param oldfile 旧文件名
    * \param newfile 新文件名
    * \return 成功返回true，否则返回false
    */
    bool Path::RenameFile(std::string oldfile, std::string newfile)
    {
        // 删除文件
        if (0 != rename(oldfile.c_str(), newfile.c_str()))
        {
            return false;
        }

        return true;
    }
}