/**
* \file random.h
* \brief 随机数封装类
*
* 类内部做了播种，提供了整数区间内获取随机值
* 和随机获取整数值的接口。
*
* Copyright (c) 2019
* All rights reserved.
*
* \version 1.0
* \author jj
*/

#ifndef __RANDOM_H__
#define __RANDOM_H__

#include "utildef.h"
#include "typedef.h"
#include <set>

namespace dtutil
{
    /**
    * \defgroup math_group 数学函数
    * math 中提供了一组负责数学计算的类。
    */	
	
    /**
    * \brief 随机数类
    * \ingroup math_group
    */
    class UTIL_EXPORT Random
    {
    public:
        /**
        * \brief 获取随机数
        * \param min 小值
        * \param max 大值
        * \return 区间随机值，包括小值和大值
        */
        static int32 RandomInt(int32 min, int32 max);

        /**
        * \brief 排除特定值获取随机数
        * \param min 小值
        * \param max 大值
        * \param exclude 排除值
        * \return 区间随机值，包括小值和大值
        */
        static int32 RandomIntExclude(int32 min, int32 max, std::set<int32> exclude);

        /**
        * \brief 生成随机数
        * \return 生成的随机值
        */
        static int32 MakeRandNum();
    };
}

#endif // __RANDOM_H__