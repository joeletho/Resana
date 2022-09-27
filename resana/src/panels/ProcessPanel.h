#pragma once

#include "Panel.h"

#include "system/processes/ProcessContainer.h"
#include "system/processes/ProcessManager.h"

#include <stack>

namespace RESANA {

enum ProcessMenu {
  View_Name = 0,
  View_Id,
  View_ParentProcessId,
  View_CpuLoad,
  View_WorkingSet,
  View_PrivateUsage,
  View_ThreadCount,
  View_PriorityClass,
  View_Status,
};

class ProcessPanel final : public Panel {
public:
  ProcessPanel();
  ProcessPanel(const ProcessPanel &other);
  ~ProcessPanel() override;

  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate(Timestep ts) override;
  void OnImGuiRender() override;

  void UpdateProcessList();
  void ShowPanel(bool *pOpen) override;
  void ShowViewMenu();
  void SetUpdateInterval(Timestep interval) override;

  [[nodiscard]] bool IsPanelOpen() const override { return mPanelOpen; }
  bool *GetMenuOption(ProcessMenu option);
  bool CheckMenuOption(ProcessMenu option);

  [[nodiscard]] uint32_t GetTableColumnCount() const;

  void SortTableEntries();
  static bool CompareWithSortSpecs(const std::shared_ptr<ProcessEntry> &lhs,
                                   const std::shared_ptr<ProcessEntry> &rhs);

private:
  void ShowProcessTable();
  void SetDefaultViewOptions();
  void SetupTableColumns();
  void CalcTableColumnCount();
  bool ShouldUpdateMemory();
  bool ShouldUpdateProcList();

  std::string GetMemoryUsageFromMap(uint32_t procId, bool update,
                                    std::function<ULONG64(uint32_t)> &&memFn);

  template <typename T> static std::string GetFormattedString(T number);

private:
  ProcessContainer mDataCache{};
  bool mPanelOpen = false;
  uint32_t mUpdateInterval{0};
  uint32_t mTableColumnCount{0};
  bool mUpdateMemoryStats = true;
  bool mUpdateProcList = true;

  TimeTick mTimeTick{};
  Timestep mLastProcListUpdate{0};
  Timestep mLastMemUpdate{0};

  std::atomic<bool> mSortData{false};

  std::unordered_map<uint32_t, std::string> mMemoryStringMap{};
  std::unordered_map<uint32_t, float> mCpuLoadMap{};
  std::unordered_map<ProcessMenu, bool> mMenuMap{};

  static const ImGuiTableSortSpecs *sCurrentSortSpecs;
};

template <typename T> std::string ProcessPanel::GetFormattedString(T number) {
  static std::stack<std::string> sStack{};
  static std::string result{};

  while (number > 0) {
    sStack.push(std::to_string((uint8_t)(number % 10)));
    number /= 10;
  }

  result.clear();
  const int length = (int)sStack.size();
  for (int i = length - 1; i >= 0; --i) {
    result.append(sStack.top());
    sStack.pop();
    if (i != 0 && (i % 3 == 0)) {
      // Insert a comma
      result.append(",");
    }
  }

  return result.length() > 0 ? result : "0";
}

} // namespace RESANA
