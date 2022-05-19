#pragma once

#include <Windows.h>
#include <Psapi.h>

#include "PerfManager.h"

namespace RESANA {

#define BYTES_PER_MB 1048576;

    class MemoryPerf {
    public:
    public:
        static void Init();

        void Run();
        void Stop();

        /* Physical Memory */
        DWORDLONG GetTotalPhys();
        DWORDLONG GetAvailPhys();
        DWORDLONG GetUsedPhys();
        SIZE_T GetCurrProcUsagePhys();

        /* Virtual Memory */
        DWORDLONG GetTotalVirtual();
        DWORDLONG GetAvailVirtual();
        DWORDLONG GetUsedVirtual();
        SIZE_T GetCurrProcUsageVirtual();

    private:
        MemoryPerf();
        ~MemoryPerf();
        void UpdateMemoryInfo();
        void UpdatePMC();
    private:
        MEMORYSTATUSEX mMemoryInfo{};
        PROCESS_MEMORY_COUNTERS_EX mPMC{};
        bool mRunning = false;

        std::thread *mInfoThread = nullptr;
        std::thread *mPMCThread = nullptr;

        static MemoryPerf *sInstance;

        friend class PerfManager;
    };

} // RESANA
