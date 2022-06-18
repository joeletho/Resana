#pragma once

#include "ConcurrentProcess.h"

#include "ProcessMap.h"
#include "ProcessEntry.h"
#include "ProcessContainer.h"

#include <queue>

namespace RESANA {

	class ProcessManager final : public ConcurrentProcess
	{
	public:
		static ProcessManager* Get();

		static void Run();
		static void Stop();

		[[nodiscard]] int GetNumProcesses() const;

		std::shared_ptr<ProcessContainer> GetData();

		void ReleaseData();

		void ResetAllRunningStatus();


	private:
		ProcessManager();
		~ProcessManager() override;

		void Terminate();

		void PrepareDataThread();
		void ProcessDataThread();

		bool PrepareData();
		ProcessContainer* GetPreparedData();
		void SetData(ProcessContainer* data);

		bool UpdateProcess(const ProcessEntry* entry) const;
		bool UpdateProcess(const PROCESSENTRY32& pe32) const;
		void SyncSelectionStatus(ProcessContainer* data) const;

		void CleanMap();
	private:
		ProcessMap mProcessMap{};
		std::shared_ptr<ProcessContainer> mProcessContainer{};

		bool mRunning = false;
		std::atomic<bool> mDataPrepared;
		std::atomic<bool> mDataReady;
		std::atomic<bool> mDataBusy;

		static ProcessManager* sInstance;

	};

}
