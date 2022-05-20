#include "rspch.h"
#include "ResourcePanel.h"

#include "perfdata/PerfManager.h"

#include <imgui.h>

namespace RESANA {

    ResourcePanel::ResourcePanel() {
        mMemoryInfo = PerfManager::GetMemory();
        mCPUInfo = PerfManager::GetCPU();
    };

    ResourcePanel::~ResourcePanel() = default;

    void ResourcePanel::ShowPanel(bool *pOpen) {

        if (ImGui::Begin("System Resources", pOpen)) {
            ImGui::TextUnformatted("Memory");
            ShowPhysicalMemoryTable();
            ShowVirtualMemoryTable();
            ShowCPUTable();
            ImGui::End();
        }
    }

    void ResourcePanel::ShowPhysicalMemoryTable() {
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

        auto total_mem = mMemoryInfo->GetTotalPhys() / BYTES_PER_MB;
        auto used_mem = mMemoryInfo->GetUsedPhys() / BYTES_PER_MB;
        float used_percent = (float) used_mem / (float) total_mem * 100.0f;
        auto avail_mem = mMemoryInfo->GetAvailPhys() / BYTES_PER_MB;
        auto proc_mem = mMemoryInfo->GetCurrProcUsagePhys() / BYTES_PER_MB;

        ImGui::Text("%llu.%llu GB", total_mem / 1000, total_mem % 10);
        ImGui::Text("%llu.%llu GB (%.1f%%)", used_mem / 1000, used_mem % 10, used_percent);
        ImGui::Text("%llu.%llu GB", avail_mem / 1000, avail_mem % 10);
        ImGui::Text("%lu MB", proc_mem);
        ImGui::EndTable();
    }

    void ResourcePanel::ShowVirtualMemoryTable() {
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

        auto total_mem = mMemoryInfo->GetTotalVirtual() / BYTES_PER_MB;
        auto used_mem = mMemoryInfo->GetUsedVirtual() / BYTES_PER_MB;
        float used_percent = (float) used_mem / (float) total_mem * 100.0f;
        auto avail_mem = mMemoryInfo->GetAvailVirtual() / BYTES_PER_MB;
        auto proc_mem = mMemoryInfo->GetCurrProcUsageVirtual() / BYTES_PER_MB;

        ImGui::Text("%llu.%llu GB", total_mem / 1000, total_mem % 10);
        ImGui::Text("%llu.%llu GB (%.1f%%)", used_mem / 1000, used_mem % 10, used_percent);
        ImGui::Text("%llu.%llu GB", avail_mem / 1000, avail_mem % 10);
        ImGui::Text("%lu MB", proc_mem);
        ImGui::EndTable();
    }

    void ResourcePanel::ShowCPUTable() {
        ImGui::BeginTable("##CPU", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("CPU");
        ImGui::TableSetupColumn("##values");
        ImGui::TableHeadersRow();
        ImGui::TableNextColumn();

        auto processorLoads = mCPUInfo->GetCurrentLoadAll();
        for (auto const &processor: processorLoads) {
            ImGui::Text("cpu %d", processor.first);
        }
        ImGui::Text("Total");
        ImGui::Text("Used by process");
        ImGui::TableNextColumn();

        static double currLoad{}, procLoad{};
        currLoad = mCPUInfo->GetCurrentLoad();
        procLoad = mCPUInfo->GetCurrentLoadProc();

        // Display values for all logical processors
        for (auto const &processor: processorLoads) {
            ImGui::Text("%.1f%%", processor.second);
        }

        // Display current CPU load and load in use by process
        ImGui::Text("%.1f%%", currLoad);
        ImGui::Text("%.1f%%", procLoad);
        ImGui::EndTable();
    }

} // RESANA