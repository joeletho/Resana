#include "PerformancePanel.h"
#include "rspch.h"

#include <imgui.h>

#include "core/Application.h"

namespace RESANA {

PerformancePanel::PerformancePanel() = default;

PerformancePanel::~PerformancePanel() = default;

void PerformancePanel::OnAttach()
{
    mUpdateInterval = TimeTick::Rate::Normal;
    mPanelOpen = false;
    InitMemoryPanel();
    InitCpuPanel();
}

void PerformancePanel::OnDetach()
{
    mPanelOpen = false;
    MemoryPerformance::Get()->Shutdown();
    mMemoryInfo = nullptr;
    CpuPerformance::Get()->Shutdown();
    mCpuInfo = nullptr;
}

void PerformancePanel::OnUpdate(Timestep ts)
{
    mUpdateInterval = (uint32_t)ts;
    if (mPanelOpen) {
        UpdateMemoryPanel();
        UpdateCpuPanel();
    }
}

void PerformancePanel::OnImGuiRender()
{
}

void PerformancePanel::ShowPanel(bool* pOpen)
{
    if ((mPanelOpen = *pOpen)) {
        if (ImGui::BeginChild("Performance", ImGui::GetContentRegionAvail())) {
            UpdateMemoryPanel();
            UpdateCpuPanel();

            ShowCpuTable();
            ImGui::TextUnformatted("Memory");
            ShowPhysicalMemoryTable();
            ShowVirtualMemoryTable();
        }

        ImGui::EndChild();
    } else {
        ClosePanels();
    }
}

void PerformancePanel::SetUpdateInterval(Timestep interval)
{
    mUpdateInterval = (uint32_t)interval;
}

void PerformancePanel::ShowPhysicalMemoryTable() const
{
    ImGui::BeginTable("##Physical Memory", 2, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Physical");
    ImGui::TableSetupColumn("##values");
    ImGui::TableHeadersRow();
    ImGui::TableNextColumn();

    // TODO: Resize columns based on size
    ImGui::Text("Total");
    ImGui::Text("In use");
    ImGui::Text("Available");
    ImGui::Text("Used by process");
    ImGui::TableNextColumn();

    const auto totalMem = mMemoryInfo->GetTotalPhysicalMB();
    const auto usedMem = mMemoryInfo->GetUsedPhysicalMB();
    const auto usedPercent = mMemoryInfo->GetMemoryLoad();
    const auto availMem = mMemoryInfo->GetAvailPhysicalMB();
    const auto procMem = MemoryPerformance::GetWorkingSetSizeMB(GetCurrentProcessId());

    ImGui::Text("%llu.%llu GB", totalMem / 1000, totalMem % 10);
    ImGui::Text("%llu.%llu GB (%.1f%%)", usedMem / 1000, usedMem % 10, usedPercent);
    ImGui::Text("%llu.%llu GB", availMem / 1000, availMem % 10);
    ImGui::Text("%llu MB", procMem);
    ImGui::EndTable();
}

void PerformancePanel::ShowVirtualMemoryTable() const
{
    ImGui::BeginTable("##Virtual Memory", 2, ImGuiTableFlags_Borders);
    ImGui::TableSetupColumn("Virtual");
    ImGui::TableSetupColumn("##values");
    ImGui::TableHeadersRow();
    ImGui::TableNextColumn();

    ImGui::Text("Total");
    ImGui::Text("In use");
    ImGui::Text("Available");
    ImGui::Text("Used by process");
    ImGui::TableNextColumn();

    const auto totalMem = mMemoryInfo->GetTotalVirtual() / BYTES_PER_MB;
    const auto usedMem = mMemoryInfo->GetUsedVirtual() / BYTES_PER_MB;
    const float usedPercent = (float)usedMem / (float)totalMem * 100.0f;
    const auto availMem = mMemoryInfo->GetAvailVirtual() / BYTES_PER_MB;
    const auto procMem = mMemoryInfo->GetCurrProcUsageVirtual() / BYTES_PER_MB;

    ImGui::Text("%llu.%llu GB", totalMem / 1000, totalMem % 10);
    ImGui::Text("%llu.%llu GB (%.1f%%)", usedMem / 1000, usedMem % 10, usedPercent);
    ImGui::Text("%llu.%llu GB", availMem / 1000, availMem % 10);
    ImGui::Text("%lu MB", procMem);
    ImGui::EndTable();
}

void PerformancePanel::ShowCpuTable()
{
    ImGui::BeginTable("##Cpu", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
    ImGui::TableSetupColumn("Cpu");
    ImGui::TableSetupColumn("##values");
    ImGui::TableHeadersRow();
    ImGui::TableNextColumn();

    mCpuInfo = CpuPerformance::Get();
    if (const auto& data = mCpuInfo->GetData()) {
        std::scoped_lock slock(data->GetMutex());

        for (const auto& p : data->GetProcessors()) {
            ImGui::Text("cpu %s", p->szName);
        }

        ImGui::Text("Total");
        ImGui::Text("Used by process");
        ImGui::TableNextColumn();

        // Display values for all logical processors
        for (const auto& p : data->GetProcessors()) {
            ImGui::Text("%.1f%%", p->FmtValue.doubleValue);
        }

        // Display current Cpu load and load in use by process
        const double currLoad = mCpuInfo->GetCurrentLoad();
        ImGui::Text("%.1f%%", currLoad);

        static long long start = Time::GetTime();
        static long long delta;
        static double procLoad = 0.0;

        delta = Time::GetTime() - start;
        if (delta > 1000) { // one second (ms)
            procLoad = mCpuInfo->GetCurrentProcessLoad();
            start = Time::GetTime();
        }
        ImGui::Text("%.1f%%", procLoad);

        mCpuInfo->ReleaseData();
    }

    ImGui::EndTable();
}

void PerformancePanel::InitCpuPanel()
{
    mCpuInfo = CpuPerformance::Get();
    mCpuInfo->SetUpdateInterval(mUpdateInterval);
}

void PerformancePanel::UpdateCpuPanel()
{
    if (!mCpuInfo) {
        InitCpuPanel();
    }

    CpuPerformance::Get()->Run();
    mCpuInfo->SetUpdateInterval(mUpdateInterval);
}

void PerformancePanel::InitMemoryPanel()
{
    mMemoryInfo = MemoryPerformance::Get();
    mMemoryInfo->SetUpdateInterval(mUpdateInterval);
}

void PerformancePanel::UpdateMemoryPanel()
{
    if (!mMemoryInfo)
        InitMemoryPanel();

    MemoryPerformance::Get()->Run();
    mMemoryInfo->SetUpdateInterval(mUpdateInterval);
}

void PerformancePanel::ClosePanels()
{
    MemoryPerformance::Get()->Stop();
    CpuPerformance::Get()->Stop();
}

} // RESANA