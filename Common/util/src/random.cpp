/**
* \file random.cpp
* \brief 随机数封装类实现代码
*/

#include "pch.h"
#include "random.h"
#include "timeutil.h"
#include <vector>


namespace dtutil
{
    /**
    * \brief 播种
    * 生成随机数需先播种，不然每次随机值都可能相同。
    * 该函数以当前时间为种子。
    */
    void Seek()
    {
        srand(TimeUtil::Tick());
    }


    /**
    * \brief 获取随机数
    * \param min 小值
    * \param max 大值
    * \return 区间随机值，包括小值和大值
    */
    int32 Random::RandomInt(int32 min, int32 max)
    {
        if (min > max)
        {
            // 交换大小
            min = min + max;
            max = min - max;    // min + max - max
            min = min - max;    // (min + max) - (min + max - max)
        }

        return min + ( MakeRandNum() % (max - min + 1) );
    }

    /**
    * \brief 排除特定值获取随机数
    * \param min 小值
    * \param max 大值
    * \param exclude 排除值
    * \return 区间随机值，包括小值和大值
    */
    int32 Random::RandomIntExclude(int32 min, int32 max, std::set<int32> exclude)
    {
        std::vector<int32> values;
        for(int32 i=min; i<=max; ++i)
        {
            if ( exclude.find(i) != exclude.end() )
            {
                continue;
            }

            values.push_back(i);
        }

        int32 count = (int32)(values.size());
        if (count == 0)
        {
            return -1;
        }

        int32 index = RandomInt(0, count-1);

        return values[index];
    }

    /**
    * \brief 生成随机数
    * \return 生成的随机值
    */
    int32 Random::MakeRandNum()
    {
        static bool flag = false;

        // 没播种，需要先播种
        if (!flag)
        {
            Seek();
            flag = true;
        }

        return rand();
    }
}