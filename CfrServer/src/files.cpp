#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "files.h"

using std::string;

string Files::cfr_base_ = "";
string Files::static_base_ = "";

void Files::Init(const std::string &cfr_dir, const std::string &static_dir)
{
    cfr_base_ = cfr_dir;
    static_base_ = static_dir;
}

const char *Files::CFRBase(void)
{
    if (cfr_base_ == "")
    {
        fprintf(stderr, "You forgot to call Files::Init()\n");
        exit(-1);
    }
    return cfr_base_.c_str();
}

const char *Files::StaticBase(void)
{
    if (static_base_ == "")
    {
        fprintf(stderr, "You forgot to call Files::Init()\n");
        exit(-1);
    }
    return static_base_.c_str();
}
