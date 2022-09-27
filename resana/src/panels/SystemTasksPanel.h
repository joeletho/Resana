#pragma once

#include "Panel.h"
#include "PerformancePanel.h"
#include "ProcessPanel.h"

#include "core/LayerStack.h"

#include "helpers/Time.h"

namespace RESANA {

class SystemTasksPanel : public Panel {
public:
    ~SystemTasksPanel() override;

    static std::shared_ptr<SystemTasksPanel> Create();

    void ShowPanel(bool* pOpen) override;

    void UpdatePanels(Timestep interval = TimeTick::Rate::Normal);
    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts = 0) override;
    void OnImGuiRender() override;

    [[nodiscard]] bool IsPanelOpen() const override { return mPanelOpen; }
    [[nodiscard]] bool ShouldClose() const;

    [[nodiscard]] static bool IsValid() { return sInstance != nullptr; }
    static void Close();

private:
    SystemTasksPanel();
    void CloseChildren();
    void ShowMenuBar() const;

    std::shared_ptr<ProcessPanel> mProcPanel = nullptr;
    std::shared_ptr<PerformancePanel> mPerfPanel = nullptr;
    LayerStack<Panel> mPanelStack {};

    bool mPanelOpen {};
    bool mShowProcPanel = true;
    bool mShowPerfPanel = false;

    uint32_t mUpdateInterval {};
    Timestep mLastTick {};
    TimeTick mTimeTick {};

    static std::shared_ptr<SystemTasksPanel> sInstance;
};

}
