#pragma once

#include "Panel.h"
#include "system/memory/MemoryPerformance.h"
#include "system/cpu/CPUPerformance.h"

//#include "helpers/Time.h"

namespace RESANA
{
	class PerformancePanel final : public Panel
	{
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
		void ShowCPUTable();
		void InitCpuPanel() const;
		void UpdateCpuPanel() const;
		void InitMemoryPanel() const;
		void UpdateMemoryPanel() const;

		static void ClosePanels();

	private:
		mutable MemoryPerformance* mMemoryInfo = nullptr;
		mutable CPUPerformance* mCPUInfo = nullptr;
		bool mPanelOpen = false;

		uint32_t mUpdateInterval{};
	};

} // RESANA
