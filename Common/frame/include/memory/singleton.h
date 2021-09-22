/**
* \file singleton.h
* \brief 单实例基类
*
* 若要声明一个单实例类，只需继承该基类即可。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __SINGLETON_H__
#define __SINGLETON_H__

#include "framedef.h"

namespace frame
{
    /**
    * \brief 单实例基类
    * \ingroup util_group
    */
    template <class T>
    class FRAME_TEMPLATE_EXPORT Singleton
    {
    public:
        /**
        * \brief 获得单实例对象
        * \return 单实例引用
        */
        static T & Instance()
        {
            return m_instance;
        }

        /**
        * \brief 获得单实例对象
        * \return 单实例对象指针
        */
        static T * GetInstance()
        {
            return &m_instance;
        }

    protected:
        /**
        * \brief 构造函数
        * 声明为protected，使该类不能直接被使用，
        * 需要继承才可使用。
        */
        Singleton() {};

        /**
        * \brief 析构函数
        * 声明为protected，使该类不能直接被使用，
        * 需要继承才可使用。
        */
        virtual ~Singleton() {};

    private:

        static T m_instance;        ///< 静态单实例对象
    };

    /// 静态单实例初始化
    template < class T > T Singleton<T>::m_instance;
}

#endif // __SINGLETON_H__