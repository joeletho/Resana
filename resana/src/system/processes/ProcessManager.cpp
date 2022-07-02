#include "ProcessManager.h"
#include "rspch.h"

#include "core/Core.h"

#include "helpers/FileUtils.h"
#include "helpers/WinFuncs.h"

#include <Windows.h>
#include <TlHelp32.h>
#include <ProcessSnapshot.h>

#include "core/Application.h"

namespace RESANA {

	ProcessManager* ProcessManager::sInstance = nullptr;

	ProcessManager::ProcessManager()
		: ConcurrentProcess("ProcessManager"), mDataReady(false), mDataBusy(false), mUpdateInterval(TimeTick::Rate::Normal)
	{
		mProcessContainer.reset(new ProcessContainer);
	}

	ProcessManager::~ProcessManager()
	{
		Time::Sleep(mUpdateInterval); // Let detached threads finish before 

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (mDataBusy) { lc.Wait(lock); }
	}

	void ProcessManager::Destroy() const
	{
		sInstance = nullptr;
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
		RS_CORE_ASSERT(IsRunning(), "Process is not currently running! Call 'ProcessManager::Run()' to start process.");

		if (!mDataReady) { return {}; } // Is data is still not ready, return nothing.

		mDataBusy = true;

		return mProcessContainer;
	}

	void ProcessManager::ReleaseData()
	{
		RS_CORE_ASSERT(mDataBusy, "Data is not owned! Did you forget to call 'GetData()'?");
		mDataBusy = false;
		auto& lc = GetLockContainer();
		lc.NotifyAll();
	}

	void ProcessManager::SetUpdateInterval(Timestep interval)
	{
		mUpdateInterval = interval;
	}

	uint32_t ProcessManager::GetUpdateSpeed() const
	{
		return mUpdateInterval;
	}

	bool ProcessManager::IsRunning() const
	{
		return mRunning;
	}

	int ProcessManager::GetNumProcesses() const
	{
		return (mProcessContainer) ? mProcessContainer->GetNumEntries() : 0;
	}

	void ProcessManager::Run()
	{
		if (!sInstance) { sInstance = Get(); }

		if (!sInstance->IsRunning())
		{
			sInstance->mRunning = true;

			auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();

			threadPool.Queue([&] { sInstance->PrepareDataThread(); });
			threadPool.Queue([&] { sInstance->ProcessDataThread(); });

		}
	}

	void ProcessManager::Stop()
	{
		if (sInstance && sInstance->IsRunning())
		{
			sInstance->mRunning = false;
			auto& lc = sInstance->GetLockContainer();
			lc.NotifyAll();
		}
	}

	void ProcessManager::Shutdown()
	{
		if (sInstance)
		{
			Stop();

			auto& app = Application::Get();
			auto& threadPool = app.GetThreadPool();
			threadPool.Queue([&]() { sInstance->Destroy(); });
			sInstance = nullptr;
		}
	}

	void ProcessManager::PrepareDataThread()
	{
		auto& lc = GetLockContainer();

		while (IsRunning())
		{
			if (PrepareData())
			{
				// Notify waiting threads
				mDataPrepared = true;
				lc.NotifyAll();
			}
			Time::Sleep(mUpdateInterval);
		}
	}

	void ProcessManager::ProcessDataThread()
	{
		while (IsRunning())
		{
			auto* data = GetPreparedData();
			SetData(data);
		}
	}

	bool ProcessManager::PrepareData()
	{
		HANDLE hProcessSnap{};
		PROCESSENTRY32 processEntry{};
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		bool error = false;
		bool snapshotReady = false;
		bool processDone = false;
		bool walkingDone = false;

		auto& app = Application::Get();
		auto& threadPool = app.GetThreadPool();

		threadPool.Queue([&] {
			snapshotReady = false;
			// Take a snapshot of all processes in the system.
			hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				PrintWin32Error("CreateToolhelp32Snapshot (of processes)");
				error = true;
			}
			snapshotReady = true;
			});

		while (!snapshotReady) {
			Time::Sleep(1);
			if (error || !IsRunning()) {
				return false;
			}
		}

		threadPool.Queue([&] {
			processDone = false;
			if (!Process32First(hProcessSnap, &processEntry))
			{
				PrintWin32Error("Process32First");
				CloseHandle(hProcessSnap); // clean the snapshot object
				error = true;
			}
			processDone = true;
			});

		ResetAllRunningStatus();

		// Now walk the snapshot of processes, and
		// get information about each process in turn

		while (!processDone) {
			Time::Sleep(1);
			if (error || !IsRunning()) {
				return false;
			}
		}

		threadPool.Queue([&, this] {
			walkingDone = false;
			do
			{
				{
					std::lock_guard lock2(mProcessMap.GetMutex());

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

			walkingDone = true;

			CloseHandle(hProcessSnap);
			});

		while (!walkingDone) {
			Time::Sleep(1);
			if (error || !IsRunning()) {
				return false;
			}
		}

		// Remove any processes not currently running
		CleanMap();

		return true;
	}

	ProcessContainer* ProcessManager::GetPreparedData()
	{
		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		while (!mDataPrepared)
		{
			if (!IsRunning()) { return nullptr; }
			lc.Wait(lock);
		}

		ProcessContainer* data = nullptr;
		lock.unlock();
		std::lock(mutex, mProcessMap.GetMutex());
		{
			std::lock_guard lock1(mutex, std::adopt_lock);
			std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

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
		lc.NotifyAll();

		return data;
	}

	void ProcessManager::SetData(ProcessContainer* data)
	{
		if (!sInstance || !data) { return; }

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);
		auto& lc = GetLockContainer();

		while (mDataBusy)
		{
			if (!IsRunning()) { return; }
			lc.Wait(lock);
		}
		lock.unlock();

		mDataReady = false;
		std::lock(mutex, lc.GetMutex());
		{
			std::lock_guard lock1(mutex, std::adopt_lock);
			std::lock_guard lock2(lc.GetMutex(), std::adopt_lock);

			mProcessContainer.reset(data);
		}
		mDataReady = true;
		lc.NotifyAll();
	}

	bool ProcessManager::UpdateProcess(const ProcessEntry* entry) const
	{
		if (!entry) { return false; }

		if (const auto& proc = mProcessMap.Find(entry->GetProcessId()))
		{
			std::mutex mutex;
			std::lock(mutex, proc->Mutex());
			{
				std::lock_guard lock(mutex, std::adopt_lock);
				std::lock_guard lock2(proc->Mutex(), std::adopt_lock);
				if (proc != entry) { // Don't update if there's no change
					proc->operator=(entry);
				}

				proc->Running() = true;
			}

			return true;
		}
		return false;
	}

	bool ProcessManager::UpdateProcess(const PROCESSENTRY32& pe32) const
	{
		if (const auto& proc = mProcessMap.Find(pe32.th32ProcessID))
		{
			std::mutex mutex;
			std::lock(mutex, proc->Mutex());
			{
				std::lock_guard lock(mutex, std::adopt_lock);
				std::lock_guard lock2(proc->Mutex(), std::adopt_lock);

				if (proc->operator!=(pe32)) { // Don't update if there's no change
					proc->operator=(pe32);
				}

				proc->Running() = true;
			}

			return true;
		}
		return false;
	}

	void ProcessManager::CleanMap()
	{
		std::mutex mutex;
		std::lock(mutex, mProcessMap.GetMutex());
		{
			std::lock_guard lock(mutex, std::adopt_lock);
			std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

			// Remove any processes not currently running
			for (auto it = mProcessMap.begin(); it != mProcessMap.end(); ++it)
			{
				if (!it->second->Running()) {
					mProcessMap.Erase(it->second);
					it = mProcessMap.begin(); // Reset iterator!
				}
			}
		}
	}

	void ProcessManager::ResetAllRunningStatus()
	{
		std::mutex mutex;
		std::lock(mutex, mProcessMap.GetMutex());
		{
			std::lock_guard lock(mutex, std::adopt_lock);
			std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

			// Set all process running status to false
			for (const auto& [id, entry] : mProcessMap)
			{
				entry->Running() = false;
			}
		}
	}

}
