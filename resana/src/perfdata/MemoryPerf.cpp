#include "MemoryPerf.h"

namespace RESANA {

    MemoryPerf *MemoryPerf::sInstance = nullptr;

    MemoryPerf::MemoryPerf() {
        mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);
    }

    MemoryPerf::~MemoryPerf() {
        mInfoThread = nullptr;
        mPMCThread = nullptr;
        sInstance = nullptr;
        delete mInfoThread, mPMCThread, sInstance;
    }

    DWORDLONG MemoryPerf::GetTotalPhys() {
        return mMemoryInfo.ullTotalPhys;
    }

    DWORDLONG MemoryPerf::GetAvailPhys() {
        return mMemoryInfo.ullAvailPhys;
    }

    DWORDLONG MemoryPerf::GetUsedPhys() {
        return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
    }

    SIZE_T MemoryPerf::GetCurrProcUsagePhys() {
        return mPMC.WorkingSetSize;
    }

    DWORDLONG MemoryPerf::GetTotalVirtual() {
        return mMemoryInfo.ullTotalPageFile;
    }

    DWORDLONG MemoryPerf::GetAvailVirtual() {
        return mMemoryInfo.ullAvailVirtual;
    }

    DWORDLONG MemoryPerf::GetUsedVirtual() {
        return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
    }

    SIZE_T MemoryPerf::GetCurrProcUsageVirtual() {
        return mPMC.PrivateUsage;
    }

    void MemoryPerf::UpdateMemoryInfo() {
        GlobalMemoryStatusEx(&mMemoryInfo);
    }

    void MemoryPerf::UpdatePMC() {
        GetProcessMemoryInfo(GetCurrentProcess(),
                             (PROCESS_MEMORY_COUNTERS *) &mPMC,
                             sizeof(mPMC));
    }

    void MemoryPerf::Init() {
        if (!sInstance) {
            sInstance = new MemoryPerf();
            sInstance->Run();
        }
    }

    void MemoryPerf::Run() {
        if (mRunning) { return; }
        mRunning = true;
        mInfoThread = new std::thread(&MemoryPerf::UpdateMemoryInfo, this);
        mInfoThread->detach();
        mPMCThread = new std::thread(&MemoryPerf::UpdatePMC, this);
        mPMCThread->detach();
    }

    void MemoryPerf::Stop() {
        mRunning = false;
    }

}
