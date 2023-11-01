#pragma once
#include "AppDef.h"

#define MajorVer 1
#define MinorVer 0
#define BuildVer 0

class Version
{
public:
    static uint32_t getFirmwareVersion(void)
    {
        return (MajorVer << 24) | (MinorVer << 16) | (BuildVer);
    }

    static const char *getFirmwareVersionString(void)
    {
        return STR(MajorVer) "." STR(MinorVer) "." STR(BuildVer);
    }
};
