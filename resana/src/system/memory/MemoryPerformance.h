#pragma once

#include "helpers/Time.h"

#include <Windows.h>
#include <Psapi.h>

namespace RESANA
{

	constexpr auto BYTES_PER_MB = 1048576;;

	class MemoryPerformance
	{
	public:
		static MemoryPerformance* Get();

		static void Start();
		static void Stop();

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

		void SetUpdateSpeed(Timestep ts = 1000);


	private:
		MemoryPerformance();
		~MemoryPerformance();
		void UpdateMemoryInfo();
		void UpdatePMC();

	private:
		MEMORYSTATUSEX mMemoryInfo{};
		PROCESS_MEMORY_COUNTERS_EX mPMC{};
		bool mRunning = false;
		uint32_t mUpdateInterval{};

		static MemoryPerformance* sInstance;
	};

} // RESANA
