#pragma once

#include "MemoryPerf.h"

#include <thread>

namespace RESANA {

    class MemoryPerf;


    class PerfManager {
    public:
        static void Init();

        std::shared_ptr<MemoryPerf> GetMemory() { return mMemoryPerf; };

        static PerfManager *Get() { return sInstance; }

    private:
        PerfManager();
        ~PerfManager();

    private:
        std::shared_ptr<MemoryPerf> mMemoryPerf;
        static PerfManager *sInstance;
    };

} // RESANA
