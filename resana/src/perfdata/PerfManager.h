#pragma once

#include "MemoryPerf.h"
#include "CPUPerf.h"

#include <thread>

namespace RESANA {

    class MemoryPerf;

    class CPUPerf;

    class PerfManager {
    public:
        static void Init();
        static MemoryPerf *GetMemory();
        static CPUPerf *GetCPU();

    private:
        PerfManager();
        ~PerfManager();

    private:
        static PerfManager *sInstance;
    };

} // RESANA
