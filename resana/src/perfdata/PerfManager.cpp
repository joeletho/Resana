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
        MemoryData::Init();
        CPUData::Init();
    }

    PerfManager::~PerfManager() {
        sInstance = nullptr;
        delete sInstance;
    }

    MemoryData *PerfManager::GetMemoryData() {
        return MemoryData::sInstance;
    }

    CPUData *PerfManager::GetCPUData() {
        return CPUData::sInstance;
    }

} // RESANA