#include "rspch.h"
#include "CPUPerformance.h"

#include "core/Core.h"
#include "core/Application.h"

#include "helpers/Container.h"

#include <mutex>

#include <PdhMsg.h>
#include <Psapi.h>
#include <Windows.h>

namespace RESANA {

	CPUPerformance* CPUPerformance::sInstance = nullptr;

	CPUPerformance::CPUPerformance()
		: ConcurrentProcess("CPUPerformance"), mUpdateInterval(TimeTick::Rate::Normal)
	{
		mLogicalCoreData.reset(new LogicalCoreData);
	}

	CPUPerformance::~CPUPerformance()
	{
		Time::Sleep(mUpdateInterval); // Let threads finish before destructing

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (mDataBusy) { lc.Wait(lock); }
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

		if (!sInstance->IsRunning())
		{
			sInstance->mRunning = true;

			const auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();

			threadPool.Queue([&] { sInstance->PrepareDataThread(); });
			threadPool.Queue([&] { sInstance->ProcessDataThread(); });
			threadPool.Queue([&] { sInstance->CalcProcessLoadThread(); });
		}
	}

	void CPUPerformance::Stop()
	{
		if (sInstance && sInstance->IsRunning())
		{
			sInstance->mRunning = false;
			auto& lc = sInstance->GetLockContainer();
			lc.NotifyAll();
		}
	}

	void CPUPerformance::Shutdown()
	{
		if (sInstance)
		{
			Stop();
			const auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();
			threadPool.Queue([&] {sInstance->Destroy(); });
			sInstance = nullptr;
		}
	}

	void CPUPerformance::Destroy() const
	{
		sInstance = nullptr;
		delete this;
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

	std::shared_ptr<LogicalCoreData> CPUPerformance::GetData()
	{
		RS_CORE_ASSERT(IsRunning(), "Process is not currently running! Call 'CPUPerformance::Run()' to start process.");

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

		return mLogicalCoreData;
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

		Time::Sleep(mUpdateInterval); // Sleep for one second.

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

	void CPUPerformance::SetUpdateInterval(Timestep interval)
	{
		mUpdateInterval = interval;
	}

	bool CPUPerformance::IsRunning() const
	{
		return mRunning;
	}

	void CPUPerformance::PrepareDataThread()
	{
		while (IsRunning())
		{
			const auto data = PrepareData();
			PushData(data);
		}
	}

	void CPUPerformance::ProcessDataThread()
	{
		while (IsRunning())
		{
			auto* data = ExtractData();
			ProcessData(data);
			SetData(data);
		}
	}

	LogicalCoreData* CPUPerformance::PrepareData() const
	{
		auto data = new LogicalCoreData();
		bool success = true;
		PdhItem* processorRef = nullptr;

		// Some counters need two samples in order to format a value, so
		// make this call to get the first value before entering the loop.
		PDH_STATUS pdhStatus = PdhCollectQueryData(mCPUCounter.Query);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			success = false;
		}

		Time::Sleep(mUpdateInterval); // Sleep for 1 second on this thread.

		pdhStatus = PdhCollectQueryData(mCPUCounter.Query);
		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			success = false;
		}


		/* FAILS HERE */
		// Get the required size of the data buffer.
		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->GetBuffer(), &data->GetSize(), processorRef);

		if (pdhStatus != PDH_MORE_DATA) {
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
			success = false;
		}

		processorRef = (PdhItem*)malloc(data->GetBuffer());
		if (!processorRef) {
			RS_CORE_ERROR("malloc for PdhGetFormattedCounterArray failed 0x{0}", pdhStatus);
			success = false;
		}

		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->GetBuffer(), &data->GetSize(), processorRef);

		if (pdhStatus != ERROR_SUCCESS) {
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
			success = false;
		}

		if (success) {
			data->SetProcessorRef(processorRef);
		}
		else
		{
			// In case of an error, free the memory
			delete data;
			data = nullptr;
		}

		return data;
	}

	void CPUPerformance::PushData(LogicalCoreData* data)
	{
		if (!data) { return; }

		std::mutex mutex;
		auto& lc = GetLockContainer();
		std::lock(mutex, lc.GetMutex());
		{
			std::lock_guard lock1(mutex, std::adopt_lock);
			std::lock_guard lock2(lc.GetMutex(), std::adopt_lock);
			mDataQueue.push(data);

		}
		lc.NotifyAll();
	}

	LogicalCoreData* CPUPerformance::ExtractData()
	{
		std::mutex mutex;
		std::unique_lock<std::mutex>lock(mutex);
		auto& lc = GetLockContainer();
		LogicalCoreData* data = nullptr;

		while (mDataQueue.empty())
		{
			if (!IsRunning()) { return nullptr; }
			lc.Wait(lock);
		}
		lock.unlock();
		std::lock(mutex, lc.GetMutex());
		{
			std::lock_guard lock1(mutex, std::adopt_lock);
			std::lock_guard lock2(lc.GetMutex(), std::adopt_lock);

			data = mDataQueue.front();
			mDataQueue.pop();
		}

		lc.NotifyAll();

		return data;
	}

	void CPUPerformance::ProcessData(LogicalCoreData* data)
	{
		if (!data) { return; }

		std::mutex mutex;
		auto& lc = GetLockContainer();

		const auto processorPtr = data->GetProcessorRef();

		// Loop through the array and add _Total to deque and cpu values into the local vector
		for (DWORD i = 0; i < data->GetSize(); ++i)
		{
			const auto processor = new PdhItem(processorPtr[i]);
			if (!processor) { continue; }

			const auto name = processor->szName;
			auto value = processor->FmtValue.doubleValue;

			if (std::strcmp(name, "_Total") == 0)
			{
				std::lock(mutex, lc.GetMutex());
				std::lock_guard lock1(mutex, std::adopt_lock);
				std::lock_guard lock2(lc.GetMutex(), std::adopt_lock);

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
				auto copy = new PdhItem(*processor);
				data->GetProcessors().emplace_back(copy);
			}
		}
	}

	void CPUPerformance::CalcProcessLoadThread()
	{
		while (mRunning) { CalcProcessLoad(); Time::Time::Sleep(mUpdateInterval); }
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

	void CPUPerformance::SetData(LogicalCoreData* data)
	{
		if (!sInstance || !data) { return; }

		SortAscending(data);

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		auto& lc = GetLockContainer();

		while (mDataBusy)
		{
			if (!IsRunning()) { return; }
			lc.Wait(lock);
		}
		lock.unlock();

		mDataReady = false;
		std::lock(mutex, lc.GetMutex());
		{
			std::lock_guard lock1(mutex, std::adopt_lock);
			std::lock_guard lock2(lc.GetMutex(), std::adopt_lock);

			mLogicalCoreData.reset(data);
		}
		mDataReady = true;
		lc.NotifyAll();
	}

	LogicalCoreData* CPUPerformance::SortAscending(LogicalCoreData* data)
	{
		auto& processors = data->GetProcessors();
		std::sort(processors.begin(), processors.end(),
			[&](const auto* left, const auto* right) {
				return std::stoi(left->szName) < std::stoi(right->szName);
			});

		return data;
	}

}