#include "PerfManager.h"

namespace RESANA {

    PerfManager *PerfManager::sInstance = nullptr;

    void PerfManager::Init() {
        PerfManager();
    }

    PerfManager::PerfManager() {
        if (!sInstance) {
            sInstance = this;
        }
        MemoryPerf::Init();
        CPUPerf::Init();
    }

    PerfManager::~PerfManager() {
        sInstance = nullptr;
        delete sInstance;
    }

    MemoryPerf *PerfManager::GetMemory() {
        return MemoryPerf::sInstance;
    }

    CPUPerf *PerfManager::GetCPU() {
        return CPUPerf::sInstance;
    }

} // RESANA