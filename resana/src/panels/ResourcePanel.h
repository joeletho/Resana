#pragma once

#include "Panel.h"
#include "perfdata/MemoryPerf.h"
#include "perfdata/CPUPerf.h"

namespace RESANA {

    class ResourcePanel : public Panel {
    public:
        ResourcePanel();
        ~ResourcePanel() override;

        void ShowPanel(bool* pOpen) override;

    private:
        void ShowPhysicalMemoryTable();
        void ShowVirtualMemoryTable();
        void ShowCPUTable();
    private:
        MemoryPerf* mMemoryInfo = nullptr;
        CPUPerf* mCPUInfo = nullptr;
    };

} // RESANA
