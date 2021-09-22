#ifndef _GLOBAL_VARIANT_H_
#define _GLOBAL_VARIANT_H_

#include <memory>
#include <string>
#include "error_code.h"

// 日志模块的一些名称
const std::string gLoading         = "Loading";
const std::string gErrorModule     = "Error";


static const int kMaxUnsignedShort = 65535U;
static const int kMaxInt = 2147483647;
static const int kMinInt = -2147483648;
static const unsigned int kMaxUnsignedInt = 4294967295U;
static const int kMaxShort = 32767;
static const int kMinShort = -32768;

#endif