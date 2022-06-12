#include "rspch.h"
#include "MemoryPerf.h"

#include "core/Application.h"

namespace RESANA {

	MemoryPerf::MemoryPerf()
	{
		mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);
		Start();
	}

	MemoryPerf::~MemoryPerf()
	{
		RS_CORE_TRACE("MemoryPerf::~MemoryPerf()");
	}

	void MemoryPerf::Start()
	{
		if (!mRunning)
		{
			mThreads.emplace_back(std::thread([&] { UpdateMemoryInfo(); }));
			mThreads.back().detach();
			mThreads.emplace_back(std::thread([&] { UpdatePMC(); }));
			mThreads.back().detach();
			mRunning = true;
		}
	}

	void MemoryPerf::Stop()
	{
		if (mRunning)
		{
			for (auto& th : mThreads) {
				if (th.joinable()) { th.join(); }
			}

			Clear(mThreads);
			mRunning = false;
		}
	}

	DWORDLONG MemoryPerf::GetTotalPhys() const
	{
		return mMemoryInfo.ullTotalPhys;
	}

	DWORDLONG MemoryPerf::GetAvailPhys() const
	{
		return mMemoryInfo.ullAvailPhys;
	}

	DWORDLONG MemoryPerf::GetUsedPhys() const
	{
		return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
	}

	SIZE_T MemoryPerf::GetCurrProcUsagePhys() const
	{
		return mPMC.WorkingSetSize;
	}

	DWORDLONG MemoryPerf::GetTotalVirtual() const
	{
		return mMemoryInfo.ullTotalPageFile;
	}

	DWORDLONG MemoryPerf::GetAvailVirtual() const
	{
		return mMemoryInfo.ullAvailVirtual;
	}

	DWORDLONG MemoryPerf::GetUsedVirtual() const
	{
		return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
	}

	SIZE_T MemoryPerf::GetCurrProcUsageVirtual() const
	{
		return mPMC.PrivateUsage;
	}

	void MemoryPerf::UpdateMemoryInfo()
	{
		while (mRunning)
		{
			if (!Application::Get().IsMinimized()) {
				GlobalMemoryStatusEx(&mMemoryInfo);
			}

			Sleep(1000);
		}
	}

	void MemoryPerf::UpdatePMC()
	{
		while (mRunning)
		{
			if (!Application::Get().IsMinimized())
			{
				GetProcessMemoryInfo(GetCurrentProcess(),
					(PROCESS_MEMORY_COUNTERS*)&mPMC, sizeof(mPMC));
			}

			Sleep(1000);
		}
	}

}
