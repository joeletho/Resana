#include "rspch.h"
#include "CPUPerformance.h"

#include "core/Core.h"
#include "core/Application.h"

#include "helpers/Container.h"

#include <PdhMsg.h>
#include <Windows.h>
#include <synchapi.h>

#include <mutex>

namespace RESANA {

	CPUPerformance* CPUPerformance::sInstance = nullptr;

	CPUPerformance::CPUPerformance() : ConcurrentProcess("CPUPerformance")
	{
		mProcessorData.reset(new ProcessorData);
	}

	CPUPerformance::~CPUPerformance()
	{
		Sleep(1000); // Let detached threads finish before destructing

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (mDataBusy) { lc.Wait(lock); }

		sInstance = nullptr;
	}

	void CPUPerformance::InitCPUData()
	{
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		mNumProcessors = (int)sysInfo.dwNumberOfProcessors;

		PDH_STATUS pdhStatus = PdhOpenQuery(nullptr, 0, &mCPUCounter.Query);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
		}

		// Specify a counter object with a wildcard for the instance.
		pdhStatus = PdhAddCounter(mCPUCounter.Query, TEXT("\\Processor(*)\\% Processor Time"), 0, &mCPUCounter.Counter);
		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
		}
	}

	void CPUPerformance::InitProcessData()
	{
		FILETIME ftime, fsys, fuser;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&mProcCounter.Last, &ftime, sizeof(FILETIME));

		mProcCounter.Handle = GetCurrentProcess();
		GetProcessTimes(mProcCounter.Handle, &ftime, &ftime, &fsys, &fuser);
		memcpy(&mProcCounter.LastSys, &fsys, sizeof(FILETIME));
		memcpy(&mProcCounter.LastUser, &fuser, sizeof(FILETIME));
	}

	void CPUPerformance::Run()
	{
		if (!sInstance) {
			sInstance = Get();
		}

		if (!sInstance->mRunning)
		{
			auto prepThread = std::thread([&] { sInstance->PrepareDataThread(); });		prepThread.detach();
			auto processThread(std::thread([&] { sInstance->ProcessDataThread(); }));	processThread.detach();
			auto calcThread(std::thread([&] { sInstance->CalcProcessLoadThread(); }));	calcThread.detach();

			sInstance->mRunning = true;
		}
	}

	void CPUPerformance::Stop()
	{
		if (sInstance && sInstance->mRunning)
		{
			sInstance->mRunning = false;

			std::thread kill([&]() { sInstance->Terminate(); }); kill.detach();
		}
	}

	void CPUPerformance::Terminate()
	{
		mRunning = false;
		this->~CPUPerformance();
	}

	CPUPerformance* CPUPerformance::Get()
	{
		if (!sInstance)
		{
			sInstance = new CPUPerformance();
			sInstance->InitCPUData();
			sInstance->InitProcessData();
		}

		return sInstance;
	}

	std::shared_ptr<ProcessorData> CPUPerformance::GetData()
	{
		RS_CORE_ASSERT(mRunning, "Process is not currently running! Call 'CPUPerformance::Run()' to start process.");

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		auto& lc = GetLockContainer();

		if (!mDataReady)
		{
			// We wait a little bit to make the UI more streamlined -- otherwise, we may
			// see some random flickering.
			lc.WaitFor(lock, std::chrono::milliseconds(10), mDataReady);
		}
		if (!mDataReady) { return {}; } // If data still isn't ready, return nothing.

		mDataBusy = true;
		lc.NotifyAll();

		return mProcessorData;
	}

	double CPUPerformance::GetAverageLoad() const
	{
		return mCPULoadAvg > 0.0 ? mCPULoadAvg : 0.0;
	}

	int CPUPerformance::GetNumProcessors() const
	{
		return mNumProcessors;
	}

	double CPUPerformance::GetCurrentLoad()
	{
		PDH_FMT_COUNTERVALUE counterValue{};

		PDH_STATUS pdhStatus = PdhOpenQuery(nullptr, 0, &mLoadCounter.Query);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
		}

		// Specify a counter object with a wildcard for the instance.
		pdhStatus = PdhAddCounter(mLoadCounter.Query,
			TEXT("\\Processor(_Total)\\% Processor Time"), 0, &mLoadCounter.Counter);
		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
		}

		pdhStatus = PdhCollectQueryData(mLoadCounter.Query);
		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		SleepEx(1000, false); // Sleep for one second.

		pdhStatus = PdhCollectQueryData(mLoadCounter.Query);
		if (pdhStatus == ERROR_SUCCESS)
		{
			pdhStatus = PdhGetFormattedCounterValue(mLoadCounter.Counter,
				PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, nullptr, &counterValue);

			if (pdhStatus == PDH_CALC_NEGATIVE_DENOMINATOR) {
				RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			}
		}
		else {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		return counterValue.doubleValue > 0.0 ? counterValue.doubleValue : 0.0;
	}

	double CPUPerformance::GetCurrentProcessLoad() const
	{
		return mProcessLoad > 0.0 ? mProcessLoad : 0.0;
	}

	void CPUPerformance::ReleaseData()
	{
		mDataBusy = false;
		auto& lc = GetLockContainer();
		lc.NotifyAll();
	}

	void CPUPerformance::PrepareDataThread()
	{
		while (mRunning) { const auto data = PrepareData(); PushData(data); }
	}

	void CPUPerformance::ProcessDataThread()
	{
		std::vector<std::thread> localThreads;

		while (mRunning)
		{
			auto data = ExtractData();
			ProcessData(data);
			localThreads.emplace_back(std::thread([&data, this]() {SetData(std::ref(data)); }));
		}

		// Join any remaining threads before terminating
		for (auto& th : localThreads)
		{
			if (th.joinable()) { th.join(); }
		}
	}

	ProcessorData* CPUPerformance::PrepareData() const
	{
		auto data = new ProcessorData();
		bool success = true;

		// Some counters need two samples in order to format a value, so
		// make this call to get the first value before entering the loop.
		PDH_STATUS pdhStatus = PdhCollectQueryData(mCPUCounter.Query);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			success = false;
		}

		Time::Sleep(1000); // Sleep for 1 second on this thread.

		pdhStatus = PdhCollectQueryData(mCPUCounter.Query);
		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			success = false;
		}

		// Get the required size of the data buffer.
		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->Buffer, &data->Size, data->ArrayRef);

		if (pdhStatus != PDH_MORE_DATA) {
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
			success = false;
		}

		data->ArrayRef = (PdhCounterValueItem*)malloc(data->Buffer);
		if (!data->ArrayRef) {
			RS_CORE_ERROR("malloc for PdhGetFormattedCounterArray failed 0x{0}", pdhStatus);
			success = false;
		}

		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->Buffer, &data->Size, data->ArrayRef);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
			success = false;
		}

		// In case of an error, free the memory
		if (!success) {
			delete data;
			data = nullptr;
		}

		return data;
	}

	void CPUPerformance::PushData(ProcessorData* data)
	{
		if (!data) { return; }

		auto& lc = GetLockContainer();

		std::scoped_lock slock(lc.GetMutex());
		mDataQueue.push(data);

		lc.NotifyAll();
	}

	ProcessorData* CPUPerformance::ExtractData()
	{
		std::mutex mutex;
		std::unique_lock<std::mutex>lock(mutex);

		auto& lc = GetLockContainer();

		while (mDataQueue.empty()) { lc.Wait(lock); }
		std::scoped_lock slock(lc.GetMutex());

		auto& data = mDataQueue.front();
		mDataQueue.pop();

		lc.NotifyAll();

		return data;
	}

	void CPUPerformance::ProcessData(ProcessorData* data)
	{
		if (!data || data->ArrayRef == nullptr) { return; }

		// Loop through the array and add _Total to deque and cpu values into the local vector
		for (DWORD i = 0; i < data->Size; ++i)
		{
			const auto processor = new PdhCounterValueItem(data->ArrayRef[i]);
			if (!processor) { continue; }

			const auto name = processor->szName;
			auto value = processor->FmtValue.doubleValue;

			if (std::strcmp(name, "_Total") == 0)
			{
				std::mutex mutex;
				std::scoped_lock slock(mutex);

				// Remove the oldest value
				if (mCPULoadValues.size() == MAX_LOAD_COUNT) {
					mCPULoadValues.pop_front();
				}

				// Add the current value and compute the average
				mCPULoadValues.push_back(value);
				mCPULoadAvg = CalculateAverage(mCPULoadValues);
			}
			else {
				// Add the processor
				data->Processors.push_back(std::make_shared<PdhCounterValueItem>(*processor));
			}
		}
	}

	void CPUPerformance::CalcProcessLoadThread()
	{
		while (mRunning) { CalcProcessLoad(); Sleep(1000); }
	}

	void CPUPerformance::CalcProcessLoad()
	{
		static FILETIME ftime, fsys, fuser;
		static ULARGE_INTEGER now, sys, user;
		static double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(mProcCounter.Handle, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));

		percent = (double)(sys.QuadPart - mProcCounter.LastSys.QuadPart) +
			(double)(user.QuadPart - mProcCounter.LastUser.QuadPart);
		percent /= (double)(now.QuadPart - mProcCounter.Last.QuadPart);
		percent /= GetNumProcessors();

		mProcCounter.Last = now;
		mProcCounter.LastUser = user;
		mProcCounter.LastSys = sys;
		mProcessLoad = percent * 100;
	}

	void CPUPerformance::SetData(ProcessorData* data)
	{
		if (!sInstance || !data) { return; }

		auto sortThread = std::thread([&data, this]() { SortAscending(std::ref(data)); });

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();

		while (mDataBusy) { lc.Wait(lock); }
		mDataReady = false;
		lc.NotifyAll(); // Notify all that the state has changed

		sortThread.join();
		mProcessorData.reset(data);

		mDataReady = true;
		lc.NotifyAll();
	}

	ProcessorData* CPUPerformance::SortAscending(ProcessorData* data)
	{
		auto& processors = data->Processors;
		std::sort(processors.begin(), processors.end(),
			[&](const std::shared_ptr<PdhCounterValueItem>& left, const std::shared_ptr<PdhCounterValueItem>& right) {
				return std::stoi(left->szName) < std::stoi(right->szName);
			});

		return data;
	}

}