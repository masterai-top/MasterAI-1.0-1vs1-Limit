#ifndef _DT_DATA_H
#define _DT_DATA_H
#include "Def.h"

class DTData
{
public:
    static int Init(const ModelParam &param);

    static int GetBdFromCode(int st,int bd);
    static int GetBdNumCodes(int st);

    static int GetBucketFromHand(int st,unsigned int hand);
    static unsigned int GetNumHands(int st);

    static bool GetIfLoadingDone();

private:
    DTData(/* args */) {}
    ~DTData() {}
};

#endif