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
        [[nodiscard]]DWORDLONG GetTotalPhys() const;
        [[nodiscard]]DWORDLONG GetAvailPhys() const;
        [[nodiscard]]DWORDLONG GetUsedPhys() const;
        [[nodiscard]]SIZE_T GetCurrProcUsagePhys() const;

        /* Virtual Memory */
        [[nodiscard]]DWORDLONG GetTotalVirtual() const;
        [[nodiscard]]DWORDLONG GetAvailVirtual() const;
        [[nodiscard]]DWORDLONG GetUsedVirtual() const;
        [[nodiscard]]SIZE_T GetCurrProcUsageVirtual() const;

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
