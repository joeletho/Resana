#include "ProcessPanel.h"
#include "rspch.h"

#include "core/Application.h"

#include "imgui/ImGuiHelpers.h"
#include <imgui.h>

#include <memory>
#include <mutex>

#include "core/Core.h"
#include "system/cpu/CpuPerformance.h"
#include "system/memory/MemoryPerformance.h"

namespace RESANA {

const ImGuiTableSortSpecs *ProcessPanel::sCurrentSortSpecs = nullptr;

ProcessPanel::ProcessPanel() = default;

ProcessPanel::ProcessPanel(const ProcessPanel &other) : Panel(other) {
  mDataCache = other.mDataCache;
  mPanelOpen = other.mPanelOpen;
  mUpdateInterval = other.mUpdateInterval;
  mTableColumnCount = other.mTableColumnCount;
  mMenuMap = other.mMenuMap;
}

ProcessPanel::~ProcessPanel() {
  mTimeTick.Stop();
  RS_CORE_TRACE("ProcessPanel destroyed");
};

void ProcessPanel::OnAttach() {
  mUpdateInterval = TimeTick::Rate::Normal;
  mPanelOpen = false;

  const auto processManager = ProcessManager::Get();
  processManager->Run();
  processManager->SetUpdateInterval(mUpdateInterval);

  mLastProcListUpdate =
      (int)mTimeTick.GetTickCount().GetMilliseconds() - (int)mUpdateInterval;

  mLastMemUpdate = (int)(mTimeTick.GetTickCount().GetMilliseconds() -
                         mUpdateInterval); // Update at first start

  SetDefaultViewOptions();
}

void ProcessPanel::OnDetach() {
  mPanelOpen = false;
  ProcessManager::Get()->Shutdown();
}

void ProcessPanel::OnUpdate(const Timestep ts) {
  mUpdateInterval = (uint32_t)ts;

  if (IsPanelOpen()) {
    const auto processManager = ProcessManager::Get();
    processManager->Run();
    processManager->SetUpdateInterval(mUpdateInterval);

    mUpdateMemoryStats = ShouldUpdateMemory();
    if (mUpdateProcList = ShouldUpdateProcList(); mUpdateProcList) {
      UpdateProcessList();
    }
  }
}

void ProcessPanel::OnImGuiRender() {}

void ProcessPanel::UpdateProcessList() {
  auto &tp = Application::Get().GetThreadPool();
  tp.Queue([&] {
    ProcessManager::SyncProcessContainer(mDataCache);
    mDataCache.UpdateEntries();
  });
}

void ProcessPanel::ShowPanel(bool *pOpen) {
  const auto processManager = ProcessManager::Get();

  if ((mPanelOpen = *pOpen)) {
    if (ImGui::BeginChild("Details", ImGui::GetContentRegionAvail())) {
      processManager->Run();
      ShowProcessTable();
    }
    ImGui::EndChild();
  } else {
    processManager->Stop();
  }
}

void ProcessPanel::ShowViewMenu() {
  // Append to parent menu bar
  ImGui::MenuItem("Process ID", nullptr, GetMenuOption(View_Id));
  ImGui::MenuItem("Parent Process ID", nullptr,
                  GetMenuOption(View_ParentProcessId));
  ImGui::MenuItem("CPU", nullptr, GetMenuOption(View_CpuLoad));
  ImGui::MenuItem("Working Set", nullptr, GetMenuOption(View_WorkingSet));
  ImGui::MenuItem("Thread Count", nullptr, GetMenuOption(View_ThreadCount));
  ImGui::MenuItem("Priority Class", nullptr, GetMenuOption(View_PriorityClass));
  ImGui::MenuItem("Status", nullptr, GetMenuOption(View_Status));

  // Update number of columns needed
  CalcTableColumnCount();
}

uint32_t ProcessPanel::GetTableColumnCount() const { return mTableColumnCount; }

bool ProcessPanel::ShouldUpdateMemory() {
  const uint32_t currentCount =
      (uint32_t)mTimeTick.GetTickCount().GetMilliseconds();
  if (const int delta =
          (int)currentCount - (int)mLastMemUpdate.GetMilliseconds();
      delta > (int)mUpdateInterval) {
    mLastMemUpdate = (int)currentCount;
    return true;
  }
  return false;
}

bool ProcessPanel::ShouldUpdateProcList() {
  const uint32_t currentCount =
      (uint32_t)mTimeTick.GetTickCount().GetMilliseconds();
  if (const int delta =
          (int)currentCount - (int)mLastProcListUpdate.GetMilliseconds();
      delta > (int)mUpdateInterval) {
    mLastProcListUpdate = (int)currentCount; // Reset counter
    return true;
  }
  return false;
}

std::string
ProcessPanel::GetMemoryUsageFromMap(const uint32_t procId, const bool update,
                                    std::function<ULONG64(uint32_t)> &&memFn) {
  std::string fString;
  if (update) {
    // Update formatted string
    fString = GetFormattedString(memFn(procId));
    if (fString.empty()) {
      fString = "0";
    }
    mMemoryStringMap[procId] = fString;
  } else {
    fString = mMemoryStringMap[procId];
  }
  return fString;
}

void ProcessPanel::SortTableEntries() {
  auto &entries = mDataCache.GetEntries();

  // Sort our data if sort specs have been changed!
  if (ImGuiTableSortSpecs *sortSpecs = ImGui::TableGetSortSpecs()) {
    if (sortSpecs->SpecsDirty || mDataCache.IsDirty()) {
      // Store in variable accessible by the sort function.
      sCurrentSortSpecs = sortSpecs;
      if (entries.size() > 1) {
        std::mutex mutex;
        std::scoped_lock slock(mutex, mDataCache.GetMutex());

        std::sort(entries.begin(), entries.end(),
                  [&](const std::shared_ptr<ProcessEntry> &lhs,
                      const std::shared_ptr<ProcessEntry> &rhs) {
                    return CompareWithSortSpecs(lhs, rhs);
                  });
      }
      sCurrentSortSpecs = nullptr;
      sortSpecs->SpecsDirty = false;
      mDataCache.SetClean();
    }
  }
}

bool ProcessPanel::CompareWithSortSpecs(
    const std::shared_ptr<ProcessEntry> &lhs,
    const std::shared_ptr<ProcessEntry> &rhs) {
  for (int n = 0; n < sCurrentSortSpecs->SpecsCount; n++) {
    const ImGuiTableColumnSortSpecs *sortSpec = &sCurrentSortSpecs->Specs[n];
    int64_t delta = 0;

    switch (sortSpec->ColumnUserID) {
    case View_Name: {
      std::string lStr = lhs->GetName();
      std::string rStr = rhs->GetName();
      // Transform to lowercase for consistent differences regardless of case
      std::transform(lStr.begin(), lStr.end(), lStr.begin(),
                     [](auto c) { return std::tolower(c); });
      std::transform(rStr.begin(), rStr.end(), rStr.begin(),
                     [](auto c) { return std::tolower(c); });
      delta = lStr.compare(rStr);
    } break;
    case View_Id:
      delta = (int)(lhs->GetId() - rhs->GetId());
      break;
    case View_ParentProcessId:
      delta = (int)(lhs->GetParentId() - rhs->GetParentId());
      break;
    case View_CpuLoad:
      // int truncates difference, so we have to assign delta based on
      //  the floating-point value.
      static double load;
      load = lhs->GetCpuLoad() - rhs->GetCpuLoad();
      if (load > 0) {
        delta = -1;
      } else if (load < 0) {
        delta = 1;
      } else {
        delta = 0;
      }
      break;
    case View_PrivateUsage:
      delta = (int64_t)(-lhs->GetPrivateUsage() + rhs->GetPrivateUsage());
      break;
    case View_WorkingSet:
      delta = (int64_t)(-lhs->GetWorkingSetSize() + rhs->GetWorkingSetSize());
      break;
    case View_ThreadCount:
      delta = (int)(-lhs->GetThreadCount() + rhs->GetThreadCount());
      break;
    case View_PriorityClass:
      delta = (int)(-lhs->GetPriorityClass() + rhs->GetPriorityClass());
      break;
    case View_Status:
      if (!lhs->IsRunning() && rhs->IsRunning()) {
        delta = -1;
      } else if (lhs->IsRunning() && !rhs->IsRunning()) {
        delta = 1;
      } else {
        delta = 0;
      }
      break;
    default:
      RS_CORE_ASSERT(false, "Unknown column!")
      break;
    }
    if (delta > 0) {
      return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true
                                                                       : false;
    }
    if (delta < 0) {
      return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false
                                                                       : true;
    }
  }

  return false;
}

void ProcessPanel::CalcTableColumnCount() {
  int numColumns = 1;
  for (const auto &[item, status] : mMenuMap) {
    numColumns += status ? 1 : 0;
  }

  mTableColumnCount = numColumns;
}

void ProcessPanel::SetUpdateInterval(Timestep interval) {
  mUpdateInterval = (uint32_t)interval;
}

bool ProcessPanel::CheckMenuOption(ProcessMenu option) {
  return mMenuMap[option];
}
bool *ProcessPanel::GetMenuOption(ProcessMenu option) {
  return &mMenuMap[option];
}

void ProcessPanel::ShowProcessTable() {
  const auto outerSize = ImVec2(-1.0f, ImGui::GetContentRegionAvail().y);

  ImGui::PushStyleColor(ImGuiCol_Text, {0.0f, 0.0f, 0.0f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, {1.0f, 1.0f, 1.0f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, {1.0f, 1.0f, 1.0f, 1.0f});
  ImGui::PushStyleColor(ImGuiCol_TableBorderLight, {1.0f, 1.0f, 1.0f, 1.0f});

  CalcTableColumnCount();

  static ImGuiTableFlags tableFlags =
      ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollX |
      ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders |
      ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
      ImGuiTableFlags_NoSavedSettings;

  if (ImGui::BeginTable("proc_table", (int)GetTableColumnCount(), tableFlags,
                        outerSize)) {

    SetupTableColumns();

    // Lock the data and read the entries
    const bool updateMemory = mUpdateMemoryStats;
    SortTableEntries();

    std::scoped_lock listLock(mDataCache.GetMutex());
    for (auto &entry : mDataCache.GetEntries()) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      {
        std::scoped_lock slock(entry->Mutex());

        static char uniqueId[64];
        sprintf_s(uniqueId, "##%u", entry->GetId());

        if (ImGui::Selectable(entry->GetName().c_str(), entry->IsSelected(),
                              ImGuiSelectableFlags_SpanAllColumns,
                              ImGui::GetColumnWidth(-1), uniqueId)) {
          mDataCache.SelectEntry(entry);
        }
        if (CheckMenuOption(View_Id)) {
          ImGui::TableNextColumn();
          ImGui::Text("%u", entry->GetId());
        }
        if (CheckMenuOption(View_ParentProcessId)) {
          ImGui::TableNextColumn();
          ImGui::Text("%u", entry->GetParentId());
        }
        if (CheckMenuOption(View_CpuLoad)) {
          ImGui::TableNextColumn();
          static double procLoad = 0.0;
          procLoad = entry->GetCpuLoad();
          ImGui::Text("%.2f%%", procLoad);
        }
        if (CheckMenuOption(View_PrivateUsage)) {
          ImGui::TableNextColumn();

          static std::string fString{};
          fString = GetMemoryUsageFromMap(entry->GetId(), updateMemory,
                                          MemoryPerformance::GetPrivateUsageKB);
          ImGui::SetRightJustify(fString.c_str());
          ImGui::Text("%s K", fString.c_str());
        }
        if (CheckMenuOption(View_WorkingSet)) {
          ImGui::TableNextColumn();

          static std::string fString{};
          fString =
              GetMemoryUsageFromMap(entry->GetId(), updateMemory,
                                    MemoryPerformance::GetWorkingSetSizeKB);
          ImGui::SetRightJustify(fString.c_str());
          ImGui::Text("%s K", fString.c_str());
        }
        if (CheckMenuOption(View_ThreadCount)) {
          ImGui::TableNextColumn();
          ImGui::Text("%u", entry->GetThreadCount());
        }
        if (CheckMenuOption(View_PriorityClass)) {
          ImGui::TableNextColumn();
          ImGui::Text("%u", entry->GetPriorityClass());
        }
        if (CheckMenuOption(View_Status)) {
          ImGui::TableNextColumn();
          ImGui::Text("%s", entry->IsRunning() ? "Running" : "Stopped");
        }
      }
    }
    ImGui::EndTable();
  }
  ImGui::PopStyleColor(4);

  mUpdateMemoryStats = false;
  mUpdateProcList = false;
}

void ProcessPanel::SetDefaultViewOptions() {
  mMenuMap[View_Id] = true;
  mMenuMap[View_CpuLoad] = true;
  mMenuMap[View_WorkingSet] = true;
  mMenuMap[View_PriorityClass] = true;
  mMenuMap[View_ThreadCount] = true;
}

void ProcessPanel::SetupTableColumns() {
  constexpr int freezeCols = 0, freezeRows = 1;
  ImGui::TableSetupScrollFreeze(freezeCols, freezeRows);
  ImGui::TableSetupColumn("Name",
                          ImGuiTableColumnFlags_DefaultSort |
                              ImGuiTableColumnFlags_WidthFixed |
                              ImGuiTableColumnFlags_NoReorder,
                          160.0f, View_Name);

  if (CheckMenuOption(View_Id)) {
    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 50.0f,
                            View_Id);
  }
  if (CheckMenuOption(View_ParentProcessId)) {
    ImGui::TableSetupColumn("PPID", ImGuiTableColumnFlags_WidthFixed, 50.0f,
                            View_ParentProcessId);
  }
  if (CheckMenuOption(View_CpuLoad)) {
    static char label[64];
    static float cpuLoad = 0.0f;
    if (mUpdateProcList) {
      cpuLoad = CpuPerformance::Get()->GetCpuLoad();
    }
    sprintf_s(label, "  %.1f%%\n  CPU", cpuLoad);
    ImGui::TableSetupColumn(label, ImGuiTableColumnFlags_WidthFixed, 0.0f,
                            View_CpuLoad);
  }
  if (CheckMenuOption(View_PrivateUsage)) {
    ImGui::TableSetupColumn("Private", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                            View_PrivateUsage);
  }
  if (CheckMenuOption(View_WorkingSet)) {
    static std::string label{};
    static uint32_t memLoad = 0;
    if (mUpdateMemoryStats) {
      memLoad = MemoryPerformance::GetMemoryLoad(0);
    }
    label = "    " + std::to_string(memLoad) + "%\nMemory";
    ImGui::TableSetupColumn(label.c_str(), ImGuiTableColumnFlags_WidthFixed,
                            0.0f, View_WorkingSet);
  }
  if (CheckMenuOption(View_ThreadCount)) {
    ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                            View_ThreadCount);
  }
  if (CheckMenuOption(View_PriorityClass)) {
    ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                            View_PriorityClass);
  }
  if (CheckMenuOption(View_Status)) {
    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 0.0f,
                            View_Status);
  }
  ImGui::TableHeadersRow();
}

} // namespace RESANA