#pragma once

#include "MemoryData.h"
// #include "CPUData.h"

namespace RESANA {

    struct SystemData {
        MemoryData Memory{};

        SystemData() = default;
        // CPUData mCPUInfo;
    };

    static SystemData SystemData{};
} // RESANA
