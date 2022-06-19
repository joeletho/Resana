#pragma once

#include "ConcurrentProcess.h"
#include "ProcessorData.h"

#include <queue>
#include <deque>

namespace RESANA {

	class CPUPerformance : public ConcurrentProcess
	{
	public:
		static void Run();
		static void Stop();

		static CPUPerformance* Get();

		std::shared_ptr<ProcessorData> GetData();

		[[nodiscard]] int GetNumProcessors() const;
		[[nodiscard]] double GetAverageLoad() const;
		[[nodiscard]] double GetCurrentLoad();
		[[nodiscard]] double GetCurrentProcessLoad() const;

		// Must be called after GetData() to unlock mutex
		void ReleaseData();

	private:
		CPUPerformance();
		~CPUPerformance() override;

		// Initializer
		void InitCPUData();
		void InitProcessData();

		// Calls destructor on new thread
		void Terminate();

		// Threads
		void PrepareDataThread();
		void ProcessDataThread();
		void CalcProcessLoadThread();

		// Called from threads
		[[nodiscard]] ProcessorData* PrepareData() const;
		ProcessorData* ExtractData();
		void CalcProcessLoad();
		void SetData(ProcessorData* data);
		void PushData(ProcessorData* data);
		void ProcessData(ProcessorData* data);

		// Helpers
		static ProcessorData* SortAscending(ProcessorData* data);

	private:
		const unsigned int MAX_LOAD_COUNT = 3;

		bool mRunning = false;
		std::atomic<bool> mDataReady;
		std::atomic<bool> mDataBusy;

		std::shared_ptr<ProcessorData> mProcessorData{};
		std::queue<ProcessorData*> mDataQueue{};
		std::deque<double> mCPULoadValues{};

		double mCPULoadAvg{};
		double mProcessLoad{};
		int mNumProcessors{};

		PDHCounter mCPUCounter{};
		PDHCounter mLoadCounter{};
		PDHCounter mProcCounter{};

		static CPUPerformance* sInstance;
	};
}
