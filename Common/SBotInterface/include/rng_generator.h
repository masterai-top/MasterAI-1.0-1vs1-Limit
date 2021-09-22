#ifndef _RNG_GENERATOR_H_
#define _RNG_GENERATOR_H_
#include "rng.h"

class RngGenerator
{
private:
    static struct drand48_data _rand_buf;
public:
    static void Init();
    static double GetRng();
private:
    RngGenerator(/* args */) {}
    ~RngGenerator(){}
};





#endif