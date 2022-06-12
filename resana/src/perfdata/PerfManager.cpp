#include "PerfManager.h"
#include <rspch.h>

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
        mMemoryPerf = std::make_shared<MemoryPerf>();
    }

    PerfManager::~PerfManager() 
    {
        sInstance = nullptr;
        delete sInstance;
    }

} // RESANA