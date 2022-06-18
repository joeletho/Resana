#pragma once

#include "ConcurrentProcess.h"

#include <queue>
#include <deque>
#include <vector>

#include <TCHAR.h>
#include <Pdh.h>

namespace RESANA {

	struct PDHCounter
	{
		HANDLE Handle{};
		HQUERY Query{};
		PDH_HCOUNTER Counter{};
		ULARGE_INTEGER Last{};
		ULARGE_INTEGER LastSys{};
		ULARGE_INTEGER LastUser{};
	};

	typedef PDH_FMT_COUNTERVALUE_ITEM PdhCounterValueItem;

	struct ProcessorData
	{
		std::vector<std::shared_ptr<PdhCounterValueItem>> Processors{};
		PdhCounterValueItem* ArrayRef = nullptr;
		DWORD Size = 0;
		DWORD Buffer = 0;
		std::mutex Mutex{};

		ProcessorData() = default;

		explicit ProcessorData(const ProcessorData* data)
			: Processors(data->Processors), ArrayRef(data->ArrayRef), Size(data->Size), Buffer(data->Buffer) {}

		~ProcessorData() {
			std::scoped_lock lock(Mutex);
		}

		std::mutex& GetMutex()
		{
			return Mutex;
		}

		void Clear()
		{
			std::scoped_lock lock(Mutex);
			Processors.clear();
		}

		ProcessorData& operator=(const ProcessorData* rhs) {
			this->Clear();
			std::scoped_lock lock(Mutex);
			for (const auto& p : rhs->Processors) {
				Processors.push_back(std::make_shared<PdhCounterValueItem>(*p));
			}
			this->ArrayRef = rhs->ArrayRef;
			this->Size = rhs->Size;
			this->Buffer = rhs->Buffer;

			return *this;
		}
	};

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
		std::atomic<bool> mDataReady = false;
		std::atomic<bool> mDataBusy = false;

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
