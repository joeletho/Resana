#pragma once

#include "Panel.h"

#include "system/processes/ProcessContainer.h"
#include "system/processes/ProcessManager.h"

namespace RESANA {

enum ProcessMenu {
    View_ProcessName = 0,
    View_ProcessId,
    View_ParentProcessId,
    View_ModuleId,
    View_MemoryUsage,
    View_ThreadCount,
    View_PriorityClass
};

class ProcessPanel final : public Panel {
public:
    ProcessPanel();
    ~ProcessPanel() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(Timestep ts) override;
    void OnImGuiRender() override;

    void UpdateProcessList();
    void ShowPanel(bool* pOpen) override;
    void ShowPanelMenu();
    void SetUpdateInterval(Timestep interval) override;

    bool IsPanelOpen() const override { return mPanelOpen; }
    bool* GetMenuOption(ProcessMenu option);
    bool CheckMenuOption(ProcessMenu option);

    [[nodiscard]] uint32_t GetTableColumnCount() const;

    void SortTableEntries();
    static bool CompareWithSortSpecs(const void* lhs, const void* rhs);

private:
    void ShowProcessTable();
    void SetDefaultViewOptions();
    void SetupTableColumns();
    void CalcTableColumnCount();

private:
    ProcessManager* mProcessManager = nullptr;
    ProcessContainer mDataCache {};
    bool mPanelOpen = false;
    uint32_t mUpdateInterval { 0 };
    uint32_t mTableColumnCount { 0 };
    std::unordered_map<ProcessMenu, bool> mMenuMap {};

	static const ImGuiTableSortSpecs* sCurrentSortSpecs;
};

} // RESANA
