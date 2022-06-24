#include "rspch.h"
#include "MemoryPerformance.h"

#include "core/Application.h"

namespace RESANA {

	MemoryPerformance* MemoryPerformance::sInstance = nullptr;

	MemoryPerformance::MemoryPerformance()
	{
		mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);
	}

	MemoryPerformance::~MemoryPerformance()
	{
		sInstance = nullptr;

		Time::Sleep((uint32_t)1000);
	}

	MemoryPerformance* MemoryPerformance::Get()
	{
		if (!sInstance)
		{
			sInstance = new MemoryPerformance();
		}

		return sInstance;
	}

	void MemoryPerformance::Start()
	{
		if (!sInstance) { MemoryPerformance::Get(); }

		if (!sInstance->IsRunning())
		{
			auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();

			threadPool.Queue([&] { sInstance->UpdateMemoryInfo(); });
			threadPool.Queue([&] { sInstance->UpdatePMC(); });

			sInstance->mRunning = true;
		}
	}

	void MemoryPerformance::Stop()
	{
		if (sInstance) {
			if (sInstance->mRunning)
			{
				sInstance->mRunning = false;

				auto& app = Application::Get();
				auto& threadPool = app.GetThreadPool();
				threadPool.Queue([&] { sInstance->~MemoryPerformance(); });
			}
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

	void MemoryPerformance::SetUpdateSpeed(Timestep ts)
	{
		mUpdateSpeed = ts;
	}

	void MemoryPerformance::UpdateMemoryInfo()
	{
		while (mRunning)
		{
			GlobalMemoryStatusEx(&mMemoryInfo);

			Sleep((uint32_t)mUpdateSpeed);
		}
	}

	void MemoryPerformance::UpdatePMC()
	{
		while (mRunning)
		{
			GetProcessMemoryInfo(GetCurrentProcess(),
				(PROCESS_MEMORY_COUNTERS*)&mPMC, sizeof(mPMC));

			Sleep((uint32_t)mUpdateSpeed);
		}
	}

}
