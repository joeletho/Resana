#pragma once

#include "system/memory/MemoryPerformance.h"

namespace RESANA {

    class MemoryPerformance;


    class PerfManager {
    public:
        static void Init();

        MemoryPerformance* GetMemory() { return mMemoryPerf; };

        static PerfManager *Get() { return sInstance; }

    private:
        PerfManager();
        ~PerfManager();

    private:
        MemoryPerformance* mMemoryPerf;
        static PerfManager *sInstance;
    };

} // RESANA
