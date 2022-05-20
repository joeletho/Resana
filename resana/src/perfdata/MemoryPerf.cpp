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
        delete mInfoThread;
        delete mPMCThread;
        delete sInstance;
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

    DWORDLONG MemoryPerf::GetTotalPhys() const {
        return mMemoryInfo.ullTotalPhys;
    }

    DWORDLONG MemoryPerf::GetAvailPhys() const {
        return mMemoryInfo.ullAvailPhys;
    }

    DWORDLONG MemoryPerf::GetUsedPhys() const {
        return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
    }

    SIZE_T MemoryPerf::GetCurrProcUsagePhys() const {
        return mPMC.WorkingSetSize;
    }

    DWORDLONG MemoryPerf::GetTotalVirtual() const {
        return mMemoryInfo.ullTotalPageFile;
    }

    DWORDLONG MemoryPerf::GetAvailVirtual() const {
        return mMemoryInfo.ullAvailVirtual;
    }

    DWORDLONG MemoryPerf::GetUsedVirtual() const {
        return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
    }

    SIZE_T MemoryPerf::GetCurrProcUsageVirtual() const {
        return mPMC.PrivateUsage;
    }

    void MemoryPerf::UpdateMemoryInfo() {
        while (mRunning) {
            GlobalMemoryStatusEx(&mMemoryInfo);
            Sleep(1000);
        }
    }

    void MemoryPerf::UpdatePMC() {
        while (mRunning) {
            GetProcessMemoryInfo(GetCurrentProcess(),
                                 (PROCESS_MEMORY_COUNTERS *) &mPMC,
                                 sizeof(mPMC));
            Sleep(1000);
        }
    }

}
