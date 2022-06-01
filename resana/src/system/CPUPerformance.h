#pragma once

#include "ConcurrentProcess.h"

#include <queue>
#include <deque>
#include <vector>
#include <mutex>

#include <TCHAR.h>
#include <Pdh.h>
#include <core/Core.h>

namespace RESANA {

	class CPUPerformance : public ConcurrentProcess {
	public:
		typedef PDH_FMT_COUNTERVALUE_ITEM counterValueItem;

		struct ProcessorData {
			std::vector<counterValueItem*> Processors{};
			counterValueItem* ArrayRef = nullptr;
			DWORD Size = 0;
			DWORD Buffer = 0;
			std::mutex Mutex{};
		};

	public:
		static CPUPerformance* Get();
		static void Release();

		virtual void Start() override;
		virtual void Stop() override;

		std::vector<counterValueItem*> GetProcessors();

		inline int GetNumProcessors() const { return mNumProcessors; };

		inline double GetAverageLoad() const { return mCPULoadAvg; };

		inline double GetCurrentProcessLoad() const { return mProcessLoad; };

		double GetCurrentLoad();

	private:
		CPUPerformance();
		virtual ~CPUPerformance();

		bool InitCPUData();
		bool InitProcessData();

		void DataPreparationThread();
		void DataExtractionThread();
		void ProcessDataThread(ProcessorData* data);
		void CalcProcessLoadThread();

		void PrepareDataFunction();
		void ExtractDataFunction();
		void CalcProcFunction();
		void SetData(ProcessorData* data);

		static std::vector<counterValueItem*> SortAscending(std::vector<counterValueItem*> vec);


		template<typename T>
		T CalculateAverage(std::deque<T>& values);


	private:
		const unsigned int MAX_LOAD_COUNT = 3;

		std::unique_ptr<ProcessorData> mProcessorData{};
		std::queue<ProcessorData*> mDataQueue{};
		std::deque<double> mCPULoadValues{};
		double mCPULoadAvg{};
		double mProcessLoad{};
		double mNumProcessors{};
		bool mRunning = false;
		bool mDataPrepared = false;

		std::vector<std::thread> mThreads{};

		std::mutex mMutex{};
		std::unique_lock<std::mutex> mLock{};

		static bool sDataInUse;
		static bool sDataReady;
		static CPUPerformance* sInstance;

		struct PDHCounter {
			HANDLE Handle{};
			HQUERY Query{};
			PDH_HCOUNTER Counter{};
			ULARGE_INTEGER Last{};
			ULARGE_INTEGER LastSys{};
			ULARGE_INTEGER LastUser{};
		};

		PDHCounter mCPUCounter;
		PDHCounter mLoadCounter;
		PDHCounter mProcCounter;
	};
}
