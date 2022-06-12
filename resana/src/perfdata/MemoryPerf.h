#pragma once

#include <Windows.h>
#include <Psapi.h>

#include "PerfManager.h"
#include <vector>
#include <thread>

namespace RESANA
{

constexpr auto BYTES_PER_MB = 1048576;;

	class MemoryPerf
	{
	public:
		MemoryPerf();
		~MemoryPerf();

		void Start();
		void Stop();

		/* Physical Memory */
		[[nodiscard]] DWORDLONG GetTotalPhys() const;
		[[nodiscard]] DWORDLONG GetAvailPhys() const;
		[[nodiscard]] DWORDLONG GetUsedPhys() const;
		[[nodiscard]] SIZE_T GetCurrProcUsagePhys() const;

		/* Virtual Memory */
		[[nodiscard]] DWORDLONG GetTotalVirtual() const;
		[[nodiscard]] DWORDLONG GetAvailVirtual() const;
		[[nodiscard]] DWORDLONG GetUsedVirtual() const;
		[[nodiscard]] SIZE_T GetCurrProcUsageVirtual() const;

		[[nodiscard]] bool IsRunning() const { return mRunning; }

	private:
		void UpdateMemoryInfo();
		void UpdatePMC();

	private:
		MEMORYSTATUSEX mMemoryInfo{};
		PROCESS_MEMORY_COUNTERS_EX mPMC{};
		bool mRunning = false;

		std::vector<std::thread> mThreads{};

		friend class PerfManager;
	};

} // RESANA
