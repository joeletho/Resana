#pragma once

#include "Windows.h"
#include "Psapi.h"

namespace RESANA {

    class MemoryData {
    public:
#define BYTES_PER_MB 1048576;
    public:
        MemoryData();
        ~MemoryData() = default;

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
        void UpdateMemoryInfo();
        void UpdatePMC();
    private:
        MEMORYSTATUSEX mMemoryInfo{};
        PROCESS_MEMORY_COUNTERS_EX mPMC{};
    };

} // RESANA
