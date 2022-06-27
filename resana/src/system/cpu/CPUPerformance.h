#pragma once

#include "system/base/ConcurrentProcess.h"
#include "LogicalCoreData.h"

#include "helpers/Time.h"

#include <queue>
#include <deque>

namespace RESANA {

	class CPUPerformance : public ConcurrentProcess
	{
	public:
		static void Run();
		static void Stop();
		static void Shutdown();

		static CPUPerformance* Get();

		std::shared_ptr<LogicalCoreData> GetData();

		[[nodiscard]] int GetNumProcessors() const;
		[[nodiscard]] double GetAverageLoad() const;
		[[nodiscard]] double GetCurrentLoad();
		[[nodiscard]] double GetCurrentProcessLoad() const;

		// Must be called after GetData() to unlock mutex
		void ReleaseData();
		void SetUpdateInterval(Timestep interval);

		bool IsRunning() const;

	private:
		CPUPerformance();
		~CPUPerformance() override;

		// Initializer
		void InitCPUData();
		void InitProcessData();

		// Calls destructor on new thread
		void Destroy() const;

		// Threads
		void PrepareDataThread();
		void ProcessDataThread();
		void CalcProcessLoadThread();

		// Called from threads
		[[nodiscard]] LogicalCoreData* PrepareData() const;
		LogicalCoreData* ExtractData();
		void CalcProcessLoad();
		void SetData(LogicalCoreData* data);
		void PushData(LogicalCoreData* data);
		void ProcessData(LogicalCoreData* data);

		// Helpers
		static LogicalCoreData* SortAscending(LogicalCoreData* data);

	private:
		const unsigned int MAX_LOAD_COUNT = 3;

		bool mRunning = false;
		uint32_t mUpdateInterval{};
		std::atomic<bool> mDataReady;
		std::atomic<bool> mDataBusy;

		std::shared_ptr<LogicalCoreData> mLogicalCoreData{};
		std::queue<LogicalCoreData*> mDataQueue{};
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
