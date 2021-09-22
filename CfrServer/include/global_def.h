#ifndef _GLOBAL_DEF_H
#define _GLOBAL_DEF_H

#include <string>

enum class CFRValueType {
  CFR_CHAR,
  CFR_SHORT,
  CFR_INT,
  CFR_DOUBLE
};

const std::string DataModuleName = "DataModule";

static const int kMaxUnsignedShort = 65535U;
static const int kMaxInt = 2147483647;
static const int kMinInt = -2147483648;
static const unsigned int kMaxUnsignedInt = 4294967295U;
static const int kMaxShort = 32767;
static const int kMinShort = -32768;

#endif