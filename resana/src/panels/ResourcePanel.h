#pragma once

#include "Panel.h"
#include "perfdata/MemoryPerf.h"
#include "system/CPUPerformance.h"

namespace RESANA
{
	class ResourcePanel : public Panel
	{
	public:
		ResourcePanel();
		~ResourcePanel() override;

		void ShowPanel(bool* pOpen) override;

	private:
		void ShowPhysicalMemoryTable();
		void ShowVirtualMemoryTable();
		void ShowCPUTable();
	private:
		std::shared_ptr<MemoryPerf> mMemoryInfo;
		CPUPerformance* mCPUInfo = nullptr;
	};

} // RESANA
