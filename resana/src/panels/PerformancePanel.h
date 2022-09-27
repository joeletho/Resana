#pragma once

#include "Panel.h"
#include "system/cpu/CpuPerformance.h"
#include "system/memory/MemoryPerformance.h"

//#include "helpers/Time.h"

namespace RESANA {
class PerformancePanel final : public Panel {
public:
    PerformancePanel();
    ~PerformancePanel() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnImGuiRender() override;
    void ShowPanel(bool* pOpen) override;
    void SetUpdateInterval(Timestep interval) override;

    bool IsPanelOpen() const override { return mPanelOpen; };

private:
    void ShowPhysicalMemoryTable() const;
    void ShowVirtualMemoryTable() const;
    void ShowCpuTable();
    void InitCpuPanel();
    void UpdateCpuPanel();
    void InitMemoryPanel();
    void UpdateMemoryPanel();

    static void ClosePanels();

private:
    std::shared_ptr<MemoryPerformance> mMemoryInfo = nullptr;
    std::shared_ptr<CpuPerformance> mCpuInfo = nullptr;
    bool mPanelOpen = false;

    uint32_t mUpdateInterval {};
};

} // RESANA
