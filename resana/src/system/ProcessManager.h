#pragma once

#include "ConcurrentProcess.h"

#include <vector>
#include <queue>
#include <thread>

#include <Windows.h>
#include <TlHelp32.h>

namespace RESANA {

	struct ProcessEntry
	{
		PROCESSENTRY32 Process{};
		bool flag = false;

		ProcessEntry() = default;

		ProcessEntry(const PROCESSENTRY32& pe32)
			: Process(pe32) {}

		~ProcessEntry() = default;

	};

	struct ProcessArray
	{
		std::vector<ProcessEntry*> Entries;

		ProcessArray() {}
		ProcessArray(ProcessArray* pArr)
			: Entries(pArr->Entries) {}

		~ProcessArray() {
			for (auto& e : Entries)
			{
				free(e);
			}
		}

		void Destroy() {
			this->~ProcessArray();
		}

		ProcessArray* operator=(const ProcessArray* rhs) {
			this->Destroy();
			*this = new ProcessArray();
			for (auto p : rhs->Entries) {
				Entries.push_back(new ProcessEntry(*p));
			}
			return this;
		}
	};

	class ProcessManager : public ConcurrentProcess
	{
	public:
		static ProcessManager* Get();

		static void Run();
		static void Stop();

		std::shared_ptr<ProcessArray> GetData();
		void ReleaseData();

		int GetNumEntries() const;

	private:
		ProcessManager();
		virtual ~ProcessManager() override;

		void Terminate();

		void ProcessAndSetDataThread();

		ProcessArray* GetProcessData();
		void SetData(ProcessArray* data);

	private:
		std::shared_ptr<ProcessArray> mProcessEntries{};
		std::queue<ProcessArray*> mProcessQueue{};
		std::vector<std::thread> mThreads{};
		bool mRunning = false;
		bool mDataReady;

		static ProcessManager* sInstance;

	};
}