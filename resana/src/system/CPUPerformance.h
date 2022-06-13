#pragma once

#include "ConcurrentProcess.h"

#include <queue>
#include <deque>
#include <vector>

#include "TCHAR.h"
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
		std::vector<PdhCounterValueItem*> Processors{};
		PdhCounterValueItem* ArrayRef = nullptr;
		DWORD Size = 0;
		DWORD Buffer = 0;

		ProcessorData() = default;
		ProcessorData(const ProcessorData* data)
			: Processors(data->Processors), ArrayRef(data->ArrayRef), Size(data->Size), Buffer(data->Buffer) {}

		~ProcessorData() {
			for (auto p : Processors) { if (p) free(p); }
		}

		void Destroy() {
			this->~ProcessorData();
		}

		ProcessorData* operator=(const ProcessorData* rhs) {
			this->Destroy();
			*this = new ProcessorData();
			for (const auto p : rhs->Processors) {
				Processors.push_back(new PdhCounterValueItem(*p));
			}
			ArrayRef = rhs->ArrayRef;
			Size = rhs->Size;
			Buffer = rhs->Buffer;

			return this;
		}
	};

	class CPUPerformance : public ConcurrentProcess
	{
	public:
		static void Run();
		static void Stop();

		static CPUPerformance* Get();

		std::shared_ptr<ProcessorData> GetData();

		int GetNumProcessors() const;
		double GetAverageLoad() const;
		double GetCurrentLoad();
		double GetCurrentProcessLoad() const;

		// Must be called after GetData() to unlock mutex
		void ReleaseData();

	private:
		CPUPerformance();
		virtual ~CPUPerformance() override;

		// Initializer
		void InitCPUData();
		void InitProcessData();

		// Calls destructor on new thread
		void Terminate();

		// Threads
		void PrepareDataThread();
		void ExtractDataThread();
		void CalcProcessLoadThread();

		// Called from threads
		[[nodiscard]] ProcessorData* PrepareData() const;
		ProcessorData* ExtractData();
		void CalcProcessLoad();
		void SetData(ProcessorData* data);
		void PushData(ProcessorData* data);
		void ProcessData(ProcessorData* data);

		// Helpers
		static std::vector<PdhCounterValueItem*>& SortAscending(std::vector<PdhCounterValueItem*>& vec);

	private:
		const unsigned int MAX_LOAD_COUNT = 3;

		bool mRunning = false;
		bool mDataReady = false;

		std::shared_ptr<ProcessorData> mProcessorData{};
		std::queue<ProcessorData*> mDataQueue{};
		std::deque<double> mCPULoadValues{};
		std::vector<std::thread> mThreads{};

		double mCPULoadAvg{};
		double mProcessLoad{};
		int mNumProcessors{};

		PDHCounter mCPUCounter{};
		PDHCounter mLoadCounter{};
		PDHCounter mProcCounter{};

		static CPUPerformance* sInstance;
	};
}
