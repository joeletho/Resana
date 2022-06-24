#include <rspch.h>
#include "PerfManager.h"

#include <memory>

namespace RESANA 
{

    PerfManager *PerfManager::sInstance = nullptr;

    void PerfManager::Init() 
    {
        if (!sInstance) {
            sInstance = new PerfManager();
        }
    }

    PerfManager::PerfManager() 
    {
        mMemoryPerf = MemoryPerformance::Get();
    }

    PerfManager::~PerfManager() 
    {
        sInstance = nullptr;
        //delete sInstance;
    }

} // RESANA