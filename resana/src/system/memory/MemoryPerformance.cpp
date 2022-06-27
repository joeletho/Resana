#include "rspch.h"
#include "MemoryPerformance.h"

#include "core/Application.h"
#include "core/Core.h"

namespace RESANA {

	MemoryPerformance* MemoryPerformance::sInstance = nullptr;

	MemoryPerformance::MemoryPerformance()
		: mPMC(), mUpdateInterval(TimeTick::Rate::Normal)
	{
	}

	MemoryPerformance::~MemoryPerformance()
	{
		// Wait for loops to stop
		Time::Sleep(mUpdateInterval);
	}

	MemoryPerformance* MemoryPerformance::Get()
	{
		if (!sInstance) {
			sInstance = new MemoryPerformance();
		}

		return sInstance;
	}

	void MemoryPerformance::Run()
	{
		if (!sInstance) { MemoryPerformance::Get(); }

		if (!sInstance->IsRunning())
		{
			sInstance->mRunning = true;

			const auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();

			threadPool.Queue([&] { sInstance->UpdateMemoryInfo(); });
			threadPool.Queue([&] { sInstance->UpdatePMC(); });
		}
	}

	void MemoryPerformance::Stop()
	{
		if (sInstance && sInstance->IsRunning()) {
			sInstance->mRunning = false;
		}
	}

	void MemoryPerformance::Shutdown()
	{
		if (sInstance)
		{
			Stop();
			auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();
			threadPool.Queue([&] { sInstance->Destroy(); });
			sInstance = nullptr;
		}
	}

	DWORDLONG MemoryPerformance::GetTotalPhys() const
	{
		return mMemoryInfo.ullTotalPhys;
	}

	DWORDLONG MemoryPerformance::GetAvailPhys() const
	{
		return mMemoryInfo.ullAvailPhys;
	}

	DWORDLONG MemoryPerformance::GetUsedPhys() const
	{
		return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
	}

	SIZE_T MemoryPerformance::GetCurrProcUsagePhys() const
	{
		return mPMC.WorkingSetSize;
	}

	DWORDLONG MemoryPerformance::GetTotalVirtual() const
	{
		return mMemoryInfo.ullTotalPageFile;
	}

	DWORDLONG MemoryPerformance::GetAvailVirtual() const
	{
		return mMemoryInfo.ullAvailVirtual;
	}

	DWORDLONG MemoryPerformance::GetUsedVirtual() const
	{
		return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
	}

	SIZE_T MemoryPerformance::GetCurrProcUsageVirtual() const
	{
		return mPMC.PrivateUsage;
	}

	void MemoryPerformance::SetUpdateInterval(Timestep interval)
	{
		mUpdateInterval = (uint32_t)interval;
	}

	void MemoryPerformance::UpdateMemoryInfo()
	{
		MEMORYSTATUSEX memInfo{};
		mMemoryInfo = memInfo;
		mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);

		do
		{
			GlobalMemoryStatusEx(&mMemoryInfo);
		} while (IsRunning() && Time::Sleep(mUpdateInterval));
		ZeroMemory(&mMemoryInfo, sizeof(MEMORYSTATUSEX));
	}

	void MemoryPerformance::UpdatePMC()
	{
		PROCESS_MEMORY_COUNTERS_EX pmc{};
		mPMC = pmc;

		const auto hProcess = GetCurrentProcess();
		do
		{
			ZeroMemory(&mPMC, sizeof(PROCESS_MEMORY_COUNTERS_EX));
			GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&mPMC, sizeof(mPMC));
		} while (IsRunning() && Time::Sleep(mUpdateInterval));

		CloseHandle(hProcess);
		ZeroMemory(&mPMC, sizeof(PROCESS_MEMORY_COUNTERS_EX));
	}

	void MemoryPerformance::Destroy() const
	{
		sInstance = nullptr;
		delete this;
	}
}
