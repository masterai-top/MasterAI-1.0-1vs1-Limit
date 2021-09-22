/**
* \file frame.h
* \brief 引用 frame 库的头文件
*
* 本头文件包含了 frame 相关的头文件，使用 frame 库的程序
* 只要包含 frame.h，即可引用 frame 中的内容，也可以仅仅
* 包含需要的头文件
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __FRAME_H__
#define __FRAME_H__

/**
* \mainpage
*
* \section brief_sec 简介
* \par
* frame 是服务器框架类，包含了 C++ 程序常用的基础类型和定义，log日志输出，
* 脚本文件读取，事件响应模块，mysql数据库处理和应用服务器基础框架
* \par
* 许多现成的 C++ 库提供了各种丰富的功能，但构建一个 C++ 工程时仍然有许多常用的
* 内容需要定义，如字符编码、始祖基类等等。
* \par
* 日志可按不同级别在控制台以不同颜色输出，方便区分，通过上层应用设置日志文件路
* 径，可将日志内容写到指定文件中，并分日期存储。
* \par
* 脚本封装了xml，ini，csv等常用配置文件的读取，同时处理了lua脚本与C++的调用。
* \par
* 事件响应模块封装了libevent相关的事件响应处理，同时提供了事件调度处理。
* \par
* 应用服务器基础框架，封装了服务器启动以及循环等待、关闭等相关的功能，上层只需
* 重载接口，处理自己的功能即可，无需关心底层运作。
* \par
* 为了减少构建工程时重复定义，用 frame 库统一封装了这些通用的内容，只要包含 
* frame.h 并链接编译生成的动态库即可。
* \par
* 用不同编译器编译的工程，需要使用同样编译器编译的库， frame 支持跨平台编译。
*
* \section content_sec 包含内容
*
* \subsection exception_sec 异常
* \par
* C++ 异常 Exception 的运用
*
* \subsection encoding_sec 字符编码
* \par
* 提供字符串编码转换
*
* \section depend_sec 依赖的库
* \par
* frame 依赖以下的库
* \li STL ＿C++标准模板库，编译器自带
* \li libpthread : libpthreadw32-2-9-1线程库
* \li libiconv : libiconv1.15字符串编码转换库
* \li libevent : libevent2.1.8轻量级的开源高性能事件通知库
* \li liblua : lua-5.1.4脚本库
* \li libtolua++ : 可以将 C++ 代码生成在 lua 中使用的接口工具库
* \li libtinyxml : 简易xml解析库
* \li libmysqlclient : mysql数据库操作库
*
* \section build_sec 编译 frame
* \par
* 必须注意：依赖的库必须是用相同的编译器编译得到的
*
* \subsection vc11_sec VC++ 2012
* \par
* 打开 vc11 目录中的工程，设置依赖库的包含和连接路径后，编译工程
*
* \subsection gcc_sec GCC
* \par
* 进入 build 目录，在 makefile 中设置依赖库的包含和连接后，执行 make
*
* \section doc_sec 代码文档
* \par 
* 本文档由 doxygen 根据代码注释自动生成，文件夹 doc 中的 doxygen.doxy 是
* frame 的 doxygen 配置文件，格式化注释的格式请参见doxygen 文档。\n
* 使用到了自动图形生成工具 Graphviz，帮助 doxygen 产生更详细的各种说明图。
*
*/

#include "framedef.h"
#include "typedef.h"

#include "exception.h"
#include "system.h"
#include "strutil.h"
#include "file.h"
#include "fileloader.h"
#include "path.h"
#include "random.h"
#include "timeutil.h"

#include "encoding.h"

#include "memory/array_pool.h"
#include "memory/singleton.h"
#include "memory/shm.h"
#include "memory/buffer.h"

#include "thread/lock.h"
#include "thread/cond.h"
#include "thread/sem.h"
#include "thread/tevent.h"
#include "thread/thread.h"
#include "thread/switchlist.h"

#include "pipe/shm_sem.h"
#include "pipe/shm_pipe.h"
#include "pipe/name_pipe.h"

#include "script.h"
#include "csv_variant.h"
#include "ini_variant.h"
#include "xml_variant.h"

#include "event/event_engine.h"
#include "app/app.h"
#include "shstd.h"

using namespace dtutil;
using namespace dtscript;

#endif // __FRAME_H__