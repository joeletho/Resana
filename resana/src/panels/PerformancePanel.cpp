#include "rspch.h"
#include "PerformancePanel.h"

#include <imgui.h>

namespace RESANA
{

	PerformancePanel::PerformancePanel()
	{
	}

	PerformancePanel::~PerformancePanel()
	{
	}

	void PerformancePanel::OnAttach()
	{
		mUpdateInterval = TimeTick::Rate::Normal;
		mPanelOpen = false;
		InitMemoryPanel();
		InitCpuPanel();
	}

	void PerformancePanel::OnDetach()
	{
		MemoryPerformance::Shutdown();
		mMemoryInfo = nullptr;
		CPUPerformance::Shutdown();
		mCPUInfo = nullptr;
	}

	void PerformancePanel::OnUpdate(Timestep ts)
	{
		if (mPanelOpen)
		{
			mUpdateInterval = (uint32_t)ts;
			UpdateMemoryPanel();
			UpdateCpuPanel();
		}
	}

	void PerformancePanel::OnImGuiRender()
	{
	}

	void PerformancePanel::ShowPanel(bool* pOpen)
	{
		if ((mPanelOpen = *pOpen))
		{
			if (ImGui::BeginChild("Performance", ImGui::GetContentRegionAvail())) \
			{
				UpdateMemoryPanel();
				UpdateCpuPanel();

				ShowCPUTable();
				ImGui::TextUnformatted("Memory");
				ShowPhysicalMemoryTable();
				ShowVirtualMemoryTable();
			}

			ImGui::EndChild();
		}
		else
		{
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

		const auto totalMem = mMemoryInfo->GetTotalPhys() / BYTES_PER_MB;
		const auto usedMem = mMemoryInfo->GetUsedPhys() / BYTES_PER_MB;
		const float usedPercent = (float)usedMem / (float)totalMem * 100.0f;
		const auto availMem = mMemoryInfo->GetAvailPhys() / BYTES_PER_MB;
		const auto procMem = mMemoryInfo->GetCurrProcUsagePhys() / BYTES_PER_MB;

		ImGui::Text("%llu.%llu GB", totalMem / 1000, totalMem % 10);
		ImGui::Text("%llu.%llu GB (%.1f%%)", usedMem / 1000, usedMem % 10, usedPercent);
		ImGui::Text("%llu.%llu GB", availMem / 1000, availMem % 10);
		ImGui::Text("%lu MB", procMem);
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

	void PerformancePanel::ShowCPUTable()
	{
		ImGui::BeginTable("##CPU", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
		ImGui::TableSetupColumn("CPU");
		ImGui::TableSetupColumn("##values");
		ImGui::TableHeadersRow();
		ImGui::TableNextColumn();

		mCPUInfo = CPUPerformance::Get();
		if (const auto& data = mCPUInfo->GetData())
		{
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

			// Display current CPU load and load in use by process
			const double currLoad = mCPUInfo->GetAverageLoad();
			const double procLoad = mCPUInfo->GetCurrentProcessLoad();
			ImGui::Text("%.1f%%", currLoad);
			ImGui::Text("%.1f%%", procLoad);

			mCPUInfo->ReleaseData();
		}

		ImGui::EndTable();
	}

	void PerformancePanel::InitCpuPanel() const
	{
		mCPUInfo = CPUPerformance::Get();
		mCPUInfo->SetUpdateInterval(mUpdateInterval);
	}

	void PerformancePanel::UpdateCpuPanel() const
	{
		if (!mCPUInfo)
			InitCpuPanel();

		CPUPerformance::Run();
		mCPUInfo->SetUpdateInterval(mUpdateInterval);
	}

	void PerformancePanel::InitMemoryPanel() const
	{
		mMemoryInfo = MemoryPerformance::Get();
		mMemoryInfo->SetUpdateInterval(mUpdateInterval);
	}

	void PerformancePanel::UpdateMemoryPanel() const
	{
		if (!mMemoryInfo)
			InitMemoryPanel();

		MemoryPerformance::Run();
		mMemoryInfo->SetUpdateInterval(mUpdateInterval);
	}

	void PerformancePanel::ClosePanels()
	{
		MemoryPerformance::Stop();
		CPUPerformance::Stop();
	}

} // RESANA