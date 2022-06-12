#include "rspch.h"
#include "CPUPerformance.h"

#include "core/Core.h"
#include "core/Application.h"

#include <Windows.h>
#include <PdhMsg.h>

#include <synchapi.h>

namespace RESANA {

	CPUPerformance* CPUPerformance::sInstance = nullptr;

	CPUPerformance::CPUPerformance() : ConcurrentProcess("CPUPerformance")
	{
		mProcessorData.reset(new ProcessorData);
	}

	CPUPerformance::~CPUPerformance()
	{
		Sleep(1000); // Let detached threads finish before destructing

		auto& lc = GetLockContainer();
		auto& writeLock = lc.GetWriteLock();
		auto& readLock = lc.GetReadLock();

		while (writeLock.owns_lock()) { lc.Wait(writeLock); }

		writeLock.lock();
		lc.Wait(writeLock, !readLock.owns_lock());

		mProcessorData->Destory();

		while (!mDataQueue.empty()) {
			auto p = mDataQueue.front();
			mDataQueue.pop();
			p->Destory();
		}

		sInstance = nullptr;
	}

	void CPUPerformance::InitCPUData()
	{
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		SYSTEM_INFO sysInfo;

		GetSystemInfo(&sysInfo);
		mNumProcessors = sysInfo.dwNumberOfProcessors;

		pdhStatus = PdhOpenQuery(nullptr, 0, &mCPUCounter.Query);
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
			sInstance->mThreads.push_back(std::thread([&] { sInstance->PrepareDataThread(); }));
			sInstance->mThreads.back().detach();
			sInstance->mThreads.push_back(std::thread([&] { sInstance->ExtractDataThread(); }));
			sInstance->mThreads.back().detach();
			sInstance->mThreads.push_back(std::thread([&] { sInstance->CalcProcessLoadThread(); }));
			sInstance->mThreads.back().detach();

			sInstance->mRunning = true;
		}
	}

	void CPUPerformance::Stop()
	{
		if (sInstance && sInstance->mRunning)
		{
			sInstance->mRunning = false;

			std::thread kill([&]() { sInstance->Terminate(); });
			kill.detach();
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

		if (!mDataReady) { return {}; }

		auto& lc = GetLockContainer();
		auto& readLock = lc.GetReadLock();
		auto& writeLock = lc.GetWriteLock();

		readLock.lock();
		lc.Wait(readLock, !writeLock.owns_lock()); // Don't read the data when it's being set!
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
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		PDH_FMT_COUNTERVALUE countervalue;

		pdhStatus = PdhOpenQuery(nullptr, 0, &mLoadCounter.Query);
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
				PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, nullptr, &countervalue);

			if (pdhStatus == PDH_CALC_NEGATIVE_DENOMINATOR) {
				RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
			}
		}
		else {
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		return countervalue.doubleValue > 0.0 ? countervalue.doubleValue : 0.0;
	}

	double CPUPerformance::GetCurrentProcessLoad() const
	{
		return mProcessLoad > 0.0 ? mProcessLoad : 0.0;
	}

	void CPUPerformance::ReleaseData()
	{
		auto& lc = GetLockContainer();
		auto& readLock = lc.GetReadLock();

		readLock.unlock();
		lc.NotifyAll();
	}

	void CPUPerformance::PrepareDataThread()
	{
		while (mRunning) { auto data = PrepareData(); PushData(data); }
	}

	void CPUPerformance::ExtractDataThread()
	{
		while (mRunning)
		{
			auto data = ExtractData();

			if (data)
			{
				ProcessData(data);
				SortAscending(data->Processors);
				SetData(data);
			}
		}
	}

	ProcessorData* CPUPerformance::PrepareData()
	{
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		auto data = new ProcessorData();
		bool success = true;

		// Some counters need two samples in order to format a value, so
		// make this call to get the first value before entering the loop.
		pdhStatus = PdhCollectQueryData(mCPUCounter.Query);
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
		auto& writeLock = lc.GetWriteLock();

		while (writeLock.owns_lock()) { lc.Wait(writeLock); }
		writeLock.lock();

		mDataQueue.push(data);

		writeLock.unlock();
		lc.NotifyAll();
	}

	ProcessorData* CPUPerformance::ExtractData()
	{
		auto& lc = GetLockContainer();
		auto& writeLock = lc.GetWriteLock();

		while (writeLock.owns_lock()) { lc.Wait(writeLock); }

		writeLock.lock();
		while (mDataQueue.empty()) { lc.Wait(writeLock); }

		auto data = mDataQueue.front();
		mDataQueue.pop();

		writeLock.unlock();
		lc.NotifyAll();

		return data;
	}

	void CPUPerformance::ProcessData(ProcessorData* data)
	{
		if (data->ArrayRef == NULL) { return; }

		// Loop through the array and add _Total to deque and cpu values into the local vector
		for (DWORD i = 0; i < data->Size; ++i)
		{
			auto processor = new PdhCounterValueItem(data->ArrayRef[i]);
			if (!processor) { continue; }

			auto name = processor->szName;
			auto value = processor->FmtValue.doubleValue;

			if (std::strcmp(name, "_Total") == 0)
			{
				// Put the total into a deque to compute the average
				if (mCPULoadValues.size() == MAX_LOAD_COUNT) {
					mCPULoadValues.pop_front();
				}

				mCPULoadValues.push_back(value);
				mCPULoadAvg = CalculateAverage(mCPULoadValues);
			}
			else {
				// Add the processor
				data->Processors.push_back(processor);
			}
		}
	}

	void CPUPerformance::CalcProcessLoadThread()
	{
		while (mRunning) { CalcProcessLoad(); Sleep(1000); }
	}

	void CPUPerformance::CalcProcessLoad()
	{
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(mProcCounter.Handle, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (double)(sys.QuadPart - mProcCounter.LastSys.QuadPart) + (double)(user.QuadPart - mProcCounter.LastUser.QuadPart);
		percent /= (double)(now.QuadPart - mProcCounter.Last.QuadPart);
		percent /= GetNumProcessors();

		mProcCounter.Last = now;
		mProcCounter.LastUser = user;
		mProcCounter.LastSys = sys;
		mProcessLoad = percent * 100;
	}

	void CPUPerformance::SetData(ProcessorData* data)
	{
		if (!data) { return; }

		auto& lc = GetLockContainer();
		auto& writeLock = lc.GetWriteLock();
		auto& readLock = lc.GetReadLock();

		while (writeLock.owns_lock()) { lc.Wait(writeLock); }
		writeLock.lock();

		mDataReady = false;
		lc.NotifyAll(); // Notify all that the state has changed

		mProcessorData->Destory();
		mProcessorData.reset(data);
		mDataReady = true;

		writeLock.unlock();
		lc.NotifyAll();
	}

	std::vector<PdhCounterValueItem*>& CPUPerformance::SortAscending(std::vector<PdhCounterValueItem*>& processors)
	{
		std::sort(processors.begin(), processors.end(), [&](PdhCounterValueItem* left, PdhCounterValueItem* right) {
			return std::stoi(left->szName) < std::stoi(right->szName); 
			});

		return processors;
	}

	template<typename T>
	T CPUPerformance::CalculateAverage(std::deque<T>& values)
	{
		T sum = 0;
		for (auto val : values) { sum += val; }

		return sum / values.size();
	}

}