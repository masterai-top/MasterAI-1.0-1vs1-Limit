#include "rng_generator.h"
#include <sys/time.h>
#include <vector>
#include <string.h>
#include <memory>
#include <sstream>
#include <iomanip>
#include "rng.h"

drand48_data RngGenerator::_rand_buf;

void RngGenerator::Init()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    srand48_r((tv.tv_sec * 1000) + (tv.tv_usec / 1000), &_rand_buf);
}

double RngGenerator::GetRng()
{        
    double rng;
    drand48_r(&_rand_buf, &rng);
    return rng;
}