/**
* \file exception.h
* \brief 包含异常基类以及异常定义宏
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <stdexcept>
#include <string>

namespace dtutil
{
    // ----------------------------------------------------
    // 异常定义
    // ----------------------------------------------------

    /**
    * C++异常
    * 提供 Exception 作为所有异常的基类，
    * 使用宏 EXCEPTION_DEF 可以很方便地定义自己的异常。
    *
    * 建议在使用异常时，统一以如下的方式抛出和捕捉:
    * \code
    *
    *   EXCEPTION_DEF(SampleException);     // 定义一个 SampleException 异常
    *
    *   ... // 其它代码
    *
    *   try
    *   {
    *       throw SampleException(str);     // 抛出异常对象
    *   }
    *   catch (Exception & e)               // 以异常对象引用的方式捕捉
    *   {
    *       report(e.what());
    *   }
    *
    * \endcode
    * 抛出的异常对象会在 catch 结束时析构，除非在 catch 中被重新抛出。
    *
    * 如果以异常对象而非引用捕捉， catch (Exception e)，
    * 会隐含地拷贝构造一个临时异常对象，并在 catch 结束时析构。
    *
    * 如果以指针的形式抛出的异常的话，则必须以指针的形式捕捉，并在不继续抛出时
    * delete 指针，否则会由于异常对象没有释放而泄露内存
    * \code
    *
    *   try
    *   {
    *       throw new SampleException(str); // 抛出异常对象指针
    *   }
    *   catch (Exception * e)               // 以指针的方式捕捉
    *   {
    *       if ( report(e->what()) )
    *           delete e;
    *       else throw;
    *   }
    *
    * \endcode
    */

    /**
    * \brief 异常基类
    * \ingroup exception_group	
    */
    class Exception : public std::runtime_error
    {
    public:
        /* 使用 const string & 的构造函数 */
        Exception(const std::string &str) : runtime_error(str) {}
        /* 返回异常名称 */
        virtual const char * GetName() const { return "Exception"; }
    };
}

/**
* \brief 异常类定义宏
* \ingroup exception_group
* \def EXCEPTION_DEF(EXCEPTION)
* \param EXCEPTION 新异常类名称
*
* 用于定义从 Exception 派生的异常
*
* \par 范例:
* \code
*   EXCEPTION_DEF(SampleException);  // SampleException 异常
*   EXCEPTION_DEF(TestException);    // TestException 异常
* \endcode
*
* \sa Exception
*/
#define EXCEPTION_DEF(EXCEPTION)\
    class EXCEPTION : public dtutil::Exception\
{\
public:\
    EXCEPTION(const std::string &str) : Exception(str) {}\
    virtual const char * GetName() const { return #EXCEPTION; }\
}

#endif // __EXCEPTION_H__
