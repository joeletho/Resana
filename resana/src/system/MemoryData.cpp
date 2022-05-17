#include "MemoryData.h"

namespace RESANA {

    MemoryData::MemoryData() {
        mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);
        UpdateMemoryInfo();
    }

    DWORDLONG MemoryData::GetTotalPhys() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullTotalPhys;
    }

    DWORDLONG MemoryData::GetAvailPhys() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullAvailPhys;
    }

    DWORDLONG MemoryData::GetUsedPhys() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
    }

    SIZE_T MemoryData::GetCurrProcUsagePhys() {
        UpdatePMC();
        return mPMC.WorkingSetSize;
    }

    DWORDLONG MemoryData::GetTotalVirtual() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullTotalPageFile;
    }

    DWORDLONG MemoryData::GetAvailVirtual() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullAvailVirtual;
    }

    DWORDLONG MemoryData::GetUsedVirtual() {
        UpdateMemoryInfo();
        return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
    }

    SIZE_T MemoryData::GetCurrProcUsageVirtual() {
        UpdatePMC();
        return mPMC.PrivateUsage;
    }

    void MemoryData::UpdateMemoryInfo() {
        GlobalMemoryStatusEx(&mMemoryInfo);
    }

    void MemoryData::UpdatePMC() {
        GetProcessMemoryInfo(GetCurrentProcess(),
                             (PROCESS_MEMORY_COUNTERS *) &mPMC,
                             sizeof(mPMC));
    }

}
