#pragma once

#include "MemoryData.h"
#include "CPUData.h"

#include <thread>

namespace RESANA {

    class PerfManager {
    public:
        static void Init();
        static MemoryData *GetMemoryData();
        static CPUData *GetCPUData();

    private:
        PerfManager();
        ~PerfManager();

    private:
        std::thread mMemThread, mCPUThread;

        static PerfManager *sInstance;
    };

} // RESANA
