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

		static void Run();
		static void Stop();
		static void Shutdown();

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

		void SetUpdateInterval(Timestep interval = TimeTick::Rate::Normal);

	private:
		MemoryPerformance();
		~MemoryPerformance();
		void UpdateMemoryInfo();
		void UpdatePMC();
		void Destroy() const;

	private:
		MEMORYSTATUSEX mMemoryInfo{};
		PROCESS_MEMORY_COUNTERS_EX mPMC{};
		uint32_t mUpdateInterval{};
		bool mRunning = false;

		static MemoryPerformance* sInstance;
	};

} // RESANA
