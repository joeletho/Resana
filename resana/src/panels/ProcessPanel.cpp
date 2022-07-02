#include "ProcessPanel.h"
#include "rspch.h"

#include "core/Application.h"

#include "imgui/ImGuiHelpers.h"
#include <imgui.h>

#include <mutex>

#include "core/Core.h"

namespace RESANA {

const ImGuiTableSortSpecs* ProcessPanel::sCurrentSortSpecs = nullptr;

ProcessPanel::ProcessPanel()
{
    SetDefaultViewOptions();
};

ProcessPanel::~ProcessPanel() = default;

void ProcessPanel::OnAttach()
{
    mUpdateInterval = TimeTick::Rate::Normal;
    mPanelOpen = false;

    mProcessManager = ProcessManager::Get();
    mProcessManager->SetUpdateInterval(mUpdateInterval);

    SetDefaultViewOptions();
}

void ProcessPanel::OnDetach()
{
    mPanelOpen = false;
    ProcessManager::Shutdown();
}

void ProcessPanel::OnUpdate(const Timestep ts)
{
    mUpdateInterval = (uint32_t)ts;

    if (IsPanelOpen()) {
        ProcessManager::Run();
        mProcessManager->SetUpdateInterval(mUpdateInterval);
        UpdateProcessList();
    }
}

void ProcessPanel::OnImGuiRender()
{
}

void ProcessPanel::UpdateProcessList()
{
    const auto& app = Application::Get();
    auto& threadPool = app.GetThreadPool();
    threadPool.Queue([&] {
        if (const auto& data = mProcessManager->GetData()) {
            if (data->GetNumEntries() > 0) {
                // Make a deep copy
                uint32_t backupId = -1;
                if (const auto selected = mDataCache.GetSelectedEntry()) {
                    backupId = selected->GetProcessId(); // Remember selected processId
                }

                mDataCache.Copy(data.get());
                mDataCache.SelectEntry(backupId); // Set selected process (if any)
                mDataCache.SetDirty();
            }
            mProcessManager->ReleaseData();
        }
    });
}

void ProcessPanel::ShowPanel(bool* pOpen)
{
    if ((mPanelOpen = *pOpen)) {
        if (ImGui::BeginChild("Details", ImGui::GetContentRegionAvail())) {
            ProcessManager::Run();
            ShowProcessTable();
        }
        ImGui::EndChild();
    } else {
        ProcessManager::Stop();
    }
}

void ProcessPanel::ShowPanelMenu()
{
    // Append to parent menu bar
    {
        ImGui::MenuItem("Process ID", nullptr, GetMenuOption(View_ProcessId));
        ImGui::MenuItem("Parent Process ID", nullptr, GetMenuOption(View_ParentProcessId));
        ImGui::MenuItem("Module ID", nullptr, GetMenuOption(View_ModuleId));
        ImGui::MenuItem("Memory Usage", nullptr, GetMenuOption(View_MemoryUsage));
        ImGui::MenuItem("Thread Count", nullptr, GetMenuOption(View_ThreadCount));
        ImGui::MenuItem("Priority Class", nullptr, GetMenuOption(View_PriorityClass));
    }

    // Update number of columns needed
    CalcTableColumnCount();
}

uint32_t ProcessPanel::GetTableColumnCount() const
{
    return mTableColumnCount;
}

void ProcessPanel::SortTableEntries()
{
    auto& entries = mDataCache.GetEntries();

    // Sort our data if sort specs have been changed!
    if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
        if (sortSpecs->SpecsDirty || mDataCache.IsDirty()) {
            sCurrentSortSpecs = sortSpecs; // Store in variable accessible by the sort function.
            if (entries.size() > 1) {
                std::sort(entries.begin(), entries.end(), [](const void* lhs, const void* rhs) {
                    return CompareWithSortSpecs(lhs, rhs);
                });
            }
            sCurrentSortSpecs = nullptr;
            sortSpecs->SpecsDirty = false;
            mDataCache.SetClean();
        }
    }
}

bool ProcessPanel::CompareWithSortSpecs(const void* lhs, const void* rhs)
{
    const auto a = (const ProcessEntry*)lhs;
    const auto b = (const ProcessEntry*)rhs;

    for (int n = 0; n < sCurrentSortSpecs->SpecsCount; n++) {
        const ImGuiTableColumnSortSpecs* sortSpec = &sCurrentSortSpecs->Specs[n];
        int delta = 0;

        switch (sortSpec->ColumnUserID) {
        case View_ProcessName: {
            std::string lStr = a->GetName();
            std::string rStr = b->GetName();
            // Transform to upper for consistent differences regardless of case
            std::transform(lStr.begin(), lStr.end(), lStr.begin(), [](auto c) { return std::tolower(c); });
            std::transform(rStr.begin(), rStr.end(), rStr.begin(), [](auto c) { return std::tolower(c); });
            delta = lStr.compare(rStr);
        } break;
        case View_ProcessId:
            delta = (int)(a->GetProcessId() - b->GetProcessId());
            break;
        case View_ParentProcessId:
            delta = (int)(a->GetParentProcessId() - b->GetParentProcessId());
            break;
        case View_ModuleId:
            delta = (int)(a->GetModuleId() - b->GetModuleId());
            break;
        case View_MemoryUsage:
            delta = (int)(a->GetMemoryUsage() - b->GetMemoryUsage());
            break;
        case View_ThreadCount:
            delta = (int)(a->GetThreadCount() - b->GetThreadCount());
            break;
        case View_PriorityClass:
            delta = (int)(a->GetPriorityClass() - b->GetPriorityClass());
            break;
        default:
            RS_CORE_ASSERT(false, "Unknown column!")
            break;
        }
        if (delta > 0) {
            return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
        }
        if (delta < 0) {
            return (sortSpec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
        }
    }

    return false;
}

void ProcessPanel::CalcTableColumnCount()
{
    int numColumns = 1;
    for (const auto& [item, status] : mMenuMap) {
        numColumns += status ? 1 : 0;
    }

    mTableColumnCount = numColumns;
}

void ProcessPanel::SetUpdateInterval(Timestep interval)
{
    mUpdateInterval = (uint32_t)interval;
}

bool ProcessPanel::CheckMenuOption(ProcessMenu option)
{
    return mMenuMap[option];
}
bool* ProcessPanel::GetMenuOption(ProcessMenu option)
{
    return &mMenuMap[option];
}

void ProcessPanel::ShowProcessTable()
{
    const auto outerSize = ImVec2(-1.0f, ImGui::GetContentRegionAvail().y);

    ImGui::PushStyleColor(ImGuiCol_Text, { 0.0f, 0.0f, 0.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_TableHeaderBg, { 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_TableBorderStrong, { 1.0f, 1.0f, 1.0f, 1.0f });
    ImGui::PushStyleColor(ImGuiCol_TableBorderLight, { 1.0f, 1.0f, 1.0f, 1.0f });

    CalcTableColumnCount();

    static ImGuiTableFlags tableFlags = ImGuiTableFlags_Sortable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_NoSavedSettings;

    if (ImGui::BeginTable("proc_table", (int)GetTableColumnCount(), tableFlags, outerSize)) {
        static bool showColWidthRow;
        showColWidthRow = true;

        SetupTableColumns();

        // Lock the data and read the entries
        std::scoped_lock dataLock(mDataCache.GetMutex());

        SortTableEntries();

        for (const auto& entry : mDataCache.GetEntries()) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            /* [DEBUG]
             * Show width of each column in the first row. */
            if (showColWidthRow) {
                ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", ImGui::GetContentRegionAvail().x);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                showColWidthRow = false; // Only called once (first row)
            }

            {
                // Lock the entry
                std::scoped_lock entryLock(entry->Mutex());

                static char uniqueId[64];
                sprintf_s(uniqueId, "##%lu", entry->GetProcessId());

                if (ImGui::Selectable(entry->GetName().c_str(), entry->IsSelected(),
                        ImGuiSelectableFlags_SpanAllColumns, ImGui::GetColumnWidth(-1), uniqueId)) {
                    mDataCache.SelectEntry(entry);
                }

                if (CheckMenuOption(View_ProcessId)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetProcessId());
                }
                if (CheckMenuOption(View_ParentProcessId)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetParentProcessId());
                }
                if (CheckMenuOption(View_ModuleId)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetModuleId());
                }
                if (CheckMenuOption(View_MemoryUsage)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetMemoryUsage());
                }
                if (CheckMenuOption(View_ThreadCount)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetThreadCount());
                }
                if (CheckMenuOption(View_PriorityClass)) {
                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", entry->GetPriorityClass());
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::PopStyleColor(4);
}

void ProcessPanel::SetDefaultViewOptions()
{
    mMenuMap[View_ProcessId] = true;
    mMenuMap[View_ThreadCount] = true;
    mMenuMap[View_PriorityClass] = true;
}

void ProcessPanel::SetupTableColumns()
{
    constexpr int freezeCols = 0, freezeRows = 1;
    ImGui::TableSetupScrollFreeze(freezeCols, freezeRows);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_PreferSortDescending | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoReorder, 160.0f, View_ProcessName);

    if (CheckMenuOption(View_ProcessId)) {
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 50.0f, View_ProcessId);
    }
    if (CheckMenuOption(View_ParentProcessId)) {
        ImGui::TableSetupColumn("PPID", ImGuiTableColumnFlags_WidthFixed, 50.0f, View_ParentProcessId);
    }
    if (CheckMenuOption(View_ModuleId)) {
        ImGui::TableSetupColumn("Module", ImGuiTableColumnFlags_WidthFixed, 50.0f, View_ModuleId);
    }
    if (CheckMenuOption(View_MemoryUsage)) {
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthFixed, 0.0f, View_MemoryUsage);
    }
    if (CheckMenuOption(View_ThreadCount)) {
        ImGui::TableSetupColumn("Threads", ImGuiTableColumnFlags_WidthFixed, 0.0f, View_ThreadCount);
    }
    if (CheckMenuOption(View_PriorityClass)) {
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 0.0f, View_PriorityClass);
    }

    ImGui::TableHeadersRow();
}

} // RESANA