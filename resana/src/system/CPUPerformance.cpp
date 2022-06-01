#include "rspch.h"
#include "CPUPerformance.h"

#include "core/Application.h"

#include <Windows.h>
#include <PdhMsg.h>

#include <synchapi.h>

namespace RESANA {

	bool CPUPerformance::sDataInUse = false;
	bool CPUPerformance::sDataReady = false;
	CPUPerformance* CPUPerformance::sInstance = nullptr;

	CPUPerformance::CPUPerformance() : ConcurrentProcess("CPUPerformance") 
	{
		mLock = std::unique_lock<std::mutex>(mMutex, std::defer_lock);
	}

	CPUPerformance::~CPUPerformance() = default;

	CPUPerformance* CPUPerformance::Get()
	{
		if (!sInstance) {
			sInstance = new CPUPerformance();
			sInstance->InitCPUData();
			sInstance->InitProcessData();
			sInstance->Start();
		}

		return sInstance;
	}

	void CPUPerformance::Release() {
		//RS_CORE_ASSERT(sInstance->mLock.owns_lock(),
		//	"CPUPerformance instance is not locked! Did you forget to call 'CPUPerformance::Get()'?");

		//sInstance->mLock.unlock();
		//sDataInUse = false;
	}

	bool CPUPerformance::InitCPUData()
	{
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		mProcessorData.reset(new ProcessorData);
		mProcessorData->Size = mNumProcessors = sysInfo.dwNumberOfProcessors;

		pdhStatus = PdhOpenQuery(nullptr, 0, &mCPUCounter.Query);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
			return false;
		}

		// Specify a counter object with a wildcard for the instance.
		pdhStatus = PdhAddCounter(mCPUCounter.Query, TEXT("\\Processor(*)\\% Processor Time"), 0, &mCPUCounter.Counter);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
			return false;
		}

		return true;
	}

	bool CPUPerformance::InitProcessData()
	{
		FILETIME ftime, fsys, fuser;
		GetSystemTimeAsFileTime(&ftime);
		memcpy(&mProcCounter.Last, &ftime, sizeof(FILETIME));

		mProcCounter.Handle = GetCurrentProcess();
		GetProcessTimes(mProcCounter.Handle, &ftime, &ftime, &fsys, &fuser);
		memcpy(&mProcCounter.LastSys, &fsys, sizeof(FILETIME));
		memcpy(&mProcCounter.LastUser, &fuser, sizeof(FILETIME));

		return true;
	}

	void CPUPerformance::Start()
	{
		if (!mRunning)
		{
			mThreads.push_back(std::thread(&CPUPerformance::DataPreparationThread, this));
			mThreads.push_back(std::thread(&CPUPerformance::DataExtractionThread, this));
			mThreads.push_back(std::thread(&CPUPerformance::CalcProcessLoadThread, this));

			mRunning = true;
		}
	}

	void CPUPerformance::Stop()
	{
		if (mRunning)
		{
			mRunning = false;

			for (auto& th : mThreads) { th.join(); }
		}
	}

	void CPUPerformance::DataPreparationThread()
	{
		while (mRunning) { PrepareDataFunction(); }
	}

	void CPUPerformance::PrepareDataFunction()
	{
		std::mutex mutex;
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		ProcessorData* data = new ProcessorData();

		// Some counters need two samples in order to format a value, so
		// make this call to get the first value before entering the loop.
		pdhStatus = PdhCollectQueryData(mCPUCounter.Query);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		Time::Sleep(1000); // Sleep for 1 second

		pdhStatus = PdhCollectQueryData(mCPUCounter.Query);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		// Get the required size of the data buffer.
		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->Buffer, &data->Size, data->ArrayRef);

		if (pdhStatus != PDH_MORE_DATA)
		{
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
		}

		data->ArrayRef = (counterValueItem*)malloc(data->Buffer);
		if (!data->ArrayRef)
		{
			RS_CORE_ERROR("malloc for PdhGetFormattedCounterArray failed 0x{0}",
				pdhStatus);
		}

		pdhStatus = PdhGetFormattedCounterArray(mCPUCounter.Counter, PDH_FMT_DOUBLE,
			&data->Buffer, &data->Size, data->ArrayRef);

		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
		}

		while (sDataInUse) { SleepEx(1, false); }

		std::lock(mutex, mMutex);
		sDataInUse = true;
		{
			std::lock_guard<std::mutex> lock1(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lock2(mMutex, std::adopt_lock);

			mDataQueue.push(data);
			mDataPrepared = true;
		}
		sDataInUse = false;
	}

	void CPUPerformance::DataExtractionThread()
	{
		while (mRunning)
		{
			if (mDataPrepared) { ExtractDataFunction(); }
			else { Sleep(100); }
		}
	}

	void CPUPerformance::ExtractDataFunction()
	{
		std::mutex mutex;
		ProcessorData* data = nullptr;

		while (sDataInUse) { Sleep(1); }

		std::lock(mutex, mMutex);
		sDataInUse = true;
		{
			std::lock_guard<std::mutex> lock1(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lock2(mMutex, std::adopt_lock);

			data = mDataQueue.front();
			mDataQueue.pop();
			mDataPrepared = false;
		}
		sDataInUse = false;

		std::thread processThread(&CPUPerformance::ProcessDataThread, this, std::ref(data));
		processThread.join();

		while (sDataInUse) { SleepEx(1, false); }

		std::lock(mutex, mMutex);
		sDataInUse = true;
		{
			std::lock_guard<std::mutex> lock1(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lock2(mMutex, std::adopt_lock);

			SetData(data);
		}
		sDataInUse = false;
	}

	void CPUPerformance::ProcessDataThread(ProcessorData* data)
	{
		if (!data) { return; }

		// Loop through the array and add _Total to deque and cpu values into the local vector
		for (DWORD i = 0; i < data->Size; ++i)
		{
			auto processor = new counterValueItem(data->ArrayRef[i]);
			auto name = processor->szName;
			auto value = processor->FmtValue.doubleValue;

			if (std::strcmp(name, "_Total") == 0)
			{
				// Put the total into a deque to compute the average
				if (mCPULoadValues.size() == MAX_LOAD_COUNT)
				{
					mCPULoadValues.pop_front();
				}

				mCPULoadValues.push_back(value);
				mCPULoadAvg = CalculateAverage(mCPULoadValues);
			}
			else
			{
				data->Processors.push_back(processor);
			}
		}

		data->Processors = SortAscending(data->Processors);
	}

	double CPUPerformance::GetCurrentLoad()
	{
		PDH_STATUS pdhStatus = ERROR_SUCCESS;
		PDH_FMT_COUNTERVALUE countervalue;

		pdhStatus = PdhOpenQuery(nullptr, 0, &mLoadCounter.Query);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
		}

		// Specify a counter object with a wildcard for the instance.
		pdhStatus = PdhAddCounter(mLoadCounter.Query,
			TEXT("\\Processor(_Total)\\% Processor Time"), 0, &mLoadCounter.Counter);
		if (pdhStatus != ERROR_SUCCESS)
		{
			RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
		}

		pdhStatus = PdhCollectQueryData(mLoadCounter.Query);
		if (pdhStatus != ERROR_SUCCESS)
		{
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
		else
		{
			RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
		}

		return countervalue.doubleValue;
	}

	void CPUPerformance::CalcProcessLoadThread()
	{
		while (mRunning)
		{
			CalcProcFunction();
			Sleep(1000);
		}
	}

	void CPUPerformance::CalcProcFunction()
	{
		FILETIME ftime, fsys, fuser;
		ULARGE_INTEGER now, sys, user;
		double percent;
		std::mutex mutex{};

		GetSystemTimeAsFileTime(&ftime);
		memcpy(&now, &ftime, sizeof(FILETIME));

		GetProcessTimes(mProcCounter.Handle, &ftime, &ftime, &fsys, &fuser);
		memcpy(&sys, &fsys, sizeof(FILETIME));
		memcpy(&user, &fuser, sizeof(FILETIME));
		percent = (double)(sys.QuadPart - mProcCounter.LastSys.QuadPart) + (double)(user.QuadPart - mProcCounter.LastUser.QuadPart);
		percent /= (double)(now.QuadPart - mProcCounter.Last.QuadPart);
		percent /= GetNumProcessors();

		while (sDataInUse) { SleepEx(1, false); }

		std::lock(mutex, mMutex);
		sDataInUse = true;
		{
			std::lock_guard<std::mutex> lock1(mutex, std::adopt_lock);
			std::lock_guard<std::mutex> lock2(mMutex, std::adopt_lock);

			mProcCounter.Last = now;
			mProcCounter.LastUser = user;
			mProcCounter.LastSys = sys;
			mProcessLoad = percent * 100;
		}
		sDataInUse = false;
	}

	void CPUPerformance::SetData(ProcessorData* data)
	{
		sDataReady = false;
		mProcessorData.reset(std::move(data));
		sDataReady = true;
	}

	std::vector<CPUPerformance::counterValueItem*> CPUPerformance::SortAscending(std::vector<counterValueItem*> processors)
	{
		std::sort(processors.begin(), processors.end(),
			[&](counterValueItem* left, counterValueItem* right)
			{
				return std::stoi(left->szName) < std::stoi(right->szName);
			});

		return processors;
	}

	std::vector<CPUPerformance::counterValueItem*> CPUPerformance::GetProcessors()
	{
		//RS_CORE_ASSERT(!mLock.owns_lock(),
		//	"CPUPerformance instance is already locked! Did you forget to call 'CPUPerformance::Release()'?");

		//while (sDataInUse && !sDataReady) { Sleep(1); }

		//sInstance->mLock;
		//sDataInUse = true;

		return mProcessorData->Processors;
	}

	template<typename T>
	T CPUPerformance::CalculateAverage(std::deque<T>& values)
	{
		T sum = 0;
		for (auto val : values) { sum += val; }
		return sum / values.size();
	}

}