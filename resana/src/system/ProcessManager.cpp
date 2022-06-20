#include "ProcessManager.h"
#include "rspch.h"

#include "core/Core.h"

#include "helpers/FileUtils.h"
#include "helpers/WinFuncs.h"

#include <Windows.h>
#include <ProcessSnapshot.h>

namespace RESANA {

	ProcessManager* ProcessManager::sInstance = nullptr;

	ProcessManager::ProcessManager()
		: ConcurrentProcess("ProcessManager"), mDataReady(false), mDataBusy(false)
	{
		mProcessContainer.reset(new ProcessContainer);
	}

	ProcessManager::~ProcessManager()
	{
		Sleep(500); // Let detached threads finish before 

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (mDataBusy) { lc.Wait(lock); }

		sInstance = nullptr;
	}

	void ProcessManager::Terminate()
	{
		mRunning = false;
		delete this;
	}

	ProcessManager* ProcessManager::Get()
	{
		if (!sInstance) {
			sInstance = new ProcessManager();
		}

		return sInstance;
	}

	std::shared_ptr<ProcessContainer> ProcessManager::GetData()
	{
		RS_CORE_ASSERT(mRunning, "Process is not currently running! Call 'ProcessManager::Run()' to start process.");

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		auto& lc = GetLockContainer();

		if (!mDataReady)
		{
			// We wait a little bit to make the UI more streamlined -- otherwise, we may
			// see some random flickering.
			lc.WaitFor(lock, std::chrono::milliseconds(20), mDataReady);
		}
		if (!mDataReady) { return {}; } // Is data is still not ready, return nothing.

		mDataBusy = true;
		lc.NotifyAll();

		return mProcessContainer;
	}

	void ProcessManager::ReleaseData()
	{
		mDataBusy = false;
		auto& lc = GetLockContainer();
		lc.NotifyAll();
	}

	int ProcessManager::GetNumProcesses() const
	{
		return (mProcessContainer) ? mProcessContainer->GetNumEntries() : 0;
	}

	void ProcessManager::Run()
	{
		if (!sInstance) { sInstance = Get(); }

		if (!sInstance->mRunning)
		{
			auto prepThread = std::thread([&] { sInstance->PrepareDataThread(); }); prepThread.detach();
			auto processThread = std::thread([&] { sInstance->ProcessDataThread(); }); processThread.detach();
			sInstance->mRunning = true;
		}
	}

	void ProcessManager::Stop()
	{
		if (sInstance && sInstance->mRunning)
		{
			sInstance->mRunning = false;
			std::thread kill([&]() { sInstance->Terminate(); }); kill.detach();
		}
	}

	void ProcessManager::PrepareDataThread()
	{
		auto& lc = GetLockContainer();

		while (mRunning)
		{
			if (PrepareData())
			{
				// Notify waiting threads	
				mDataPrepared = true;
				lc.NotifyAll();
			}
			Time::Sleep(1000);
		}
	}

	void ProcessManager::ProcessDataThread()
	{
		std::vector<std::thread> localThreads;

		while (mRunning)
		{
			auto data = GetPreparedData();
			localThreads.emplace_back(std::thread([&data, this]() {SetData(std::ref(data)); }));
		}

		// Join any remaining threads before terminating
		for (auto& th : localThreads)
		{
			if (th.joinable()) { th.join(); }
		}
	}

	bool ProcessManager::PrepareData()
	{
		PROCESSENTRY32 processEntry{};
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		// Take a snapshot of all processes in the system.
		HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			PrintWin32Error("CreateToolhelp32Snapshot (of processes)");
			return false;
		}

		if (!Process32First(hProcessSnap, &processEntry)) {
			PrintWin32Error("Process32First");
			CloseHandle(hProcessSnap);          // clean the snapshot object
			return false;
		}

		ResetAllRunningStatus();

		// Now walk the snapshot of processes, and
		// get information about each process in turn
		do
		{
			{
				std::scoped_lock slock(mProcessMap.GetMutex());

				// Set process running status to true and
				//	update process, if applicable
				if (!UpdateProcess(processEntry))
				{
					// Otherwise, add new process
					mProcessMap.Emplace(new ProcessEntry(processEntry));
				}
			}

			/* TODO: Implement these features
			// List the modules and threads associated with this process
			ListProcessModules(procEntry.th32ProcessID);
			ListProcessThreads(procEntry.th32ProcessID);
			*/

		} while (Process32Next(hProcessSnap, &processEntry));

		CloseHandle(hProcessSnap);

		// Remove any processes not currently running
		CleanMap();

		return true;
	}

	ProcessContainer* ProcessManager::GetPreparedData()
	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (!mDataPrepared) { lc.Wait(lock); }

		ProcessContainer* data = nullptr;
		{
			std::scoped_lock slock(mProcessMap.GetMutex());

			// Add a deep copy to the ProcessContainer to preserve mProcessMap data
			data = new ProcessContainer();
			for (auto& [id, entry] : mProcessMap)
			{
				auto* copy = new ProcessEntry(entry);
				data->AddEntry(copy);
			}
		}
		// Unset the flag
		mDataPrepared = false;

		return data;
	}

	void ProcessManager::SetData(ProcessContainer* data)
	{
		if (!sInstance || !data) { return; }

		// Update selected entry before submitting.
		auto syncThread = std::thread([&data, this]() { SyncSelectionStatus(std::ref(data)); });

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (mDataBusy) { lc.Wait(lock); }

		mDataReady = false;
		lc.NotifyAll(); // Notify all that the state has changed

		syncThread.join();

		// Reset process entries
		mProcessContainer.reset(data);

		mDataReady = true;
		lc.NotifyAll();
	}

	bool ProcessManager::UpdateProcess(const ProcessEntry* entry) const
	{
		if (!entry) { return false; }

		if (const auto& proc = mProcessMap.Find(entry->GetProcessId()))
		{
			std::scoped_lock slock(proc->Mutex());

			if (proc != entry) { // Don't update if there's no change
				proc->operator=(entry);
			}

			proc->Running() = true;

			return true;
		}
		return false;
	}

	bool ProcessManager::UpdateProcess(const PROCESSENTRY32& pe32) const
	{
		if (const auto& proc = mProcessMap.Find(pe32.th32ProcessID))
		{
			std::scoped_lock slock(proc->Mutex());

			if (proc->operator!=(pe32)) { // Don't update if there's no change
				proc->operator=(pe32);
			}

			proc->Running() = true;

			return true;
		}
		return false;
	}

	void ProcessManager::SyncSelectionStatus(ProcessContainer* data) const
	{
		if (const auto& selectedEntry = mProcessContainer->GetSelectedEntry())
		{
			const auto& entries = data->GetEntries();
			for (auto& entry : entries)
			{
				if (entry->GetProcessId() == selectedEntry->GetProcessId()) {
					data->SelectEntry(entry);
					break;
				}
			}
		}
	}

	void ProcessManager::CleanMap()
	{
		std::scoped_lock slock(mProcessMap.GetMutex());

		// Remove any processes not currently running
		for (auto it = mProcessMap.begin(); it != mProcessMap.end(); ++it)
		{
			if (!it->second->Running()) {
				mProcessMap.Erase(it->second);
				it = mProcessMap.begin(); // Reset iterator!
			}
		}
	}

	void ProcessManager::ResetAllRunningStatus()
	{
		// Set all process running status to false
		std::scoped_lock slock(mProcessMap.GetMutex());
		for (const auto& [id, entry] : mProcessMap)
		{
			entry->Running() = false;
		}
	}

}