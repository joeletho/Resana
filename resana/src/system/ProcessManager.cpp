#include "ProcessManager.h"
#include "rspch.h"

#include "core/Core.h"

#include "helpers/FileUtils.h"
#include "helpers/WinFuncs.h"

#include <ProcessSnapshot.h>

namespace RESANA {

	ProcessManager* ProcessManager::sInstance = nullptr;

	ProcessManager::ProcessManager() : ConcurrentProcess("ProcessManager")
	{
		mProcessEntries.reset(new ProcessArray);
	}

	ProcessManager::~ProcessManager()
	{
		Sleep(1000); // Let detached threads finish before destructing

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		auto& writeLock = lc.GetWriteLock();
		auto& readLock = lc.GetReadLock();

		while (writeLock.owns_lock()) { lc.Wait(lock); }

		writeLock.lock();
		lc.Wait(writeLock, !readLock.owns_lock());

		mProcessEntries->Destroy();

		while (!mProcessQueue.empty())
		{
			auto p = mProcessQueue.front();
			mProcessQueue.pop();

			sInstance = nullptr;
		}
	}

	void ProcessManager::Terminate()
	{
		mRunning = false;
		this->~ProcessManager();
	}

	ProcessManager* ProcessManager::Get()
	{
		if (!sInstance) {
			sInstance = new ProcessManager();
		}

		return sInstance;
	}

	std::shared_ptr<ProcessArray> ProcessManager::GetData()
	{
		RS_CORE_ASSERT(mRunning, "Process is not currently running! Call 'ProcessManager::Run()' to start process.");

		if (!mDataReady) { return {}; }

		auto& lc = GetLockContainer();
		auto& readLock = lc.GetReadLock();
		auto& writeLock = lc.GetWriteLock();

		readLock.lock();
		lc.Wait(readLock, !writeLock.owns_lock()); // Don't read the data when it's being set!
		lc.NotifyAll();

		return mProcessEntries;
	}

	void ProcessManager::ReleaseData()
	{
		auto& lc = GetLockContainer();
		auto& readLock = lc.GetReadLock();

		readLock.unlock();
		lc.NotifyAll();
	}
	int ProcessManager::GetNumEntries() const
	{
		return (mProcessEntries) ? mProcessEntries->Entries.size() : 0;
	}

	void ProcessManager::Run()
	{
		if (!sInstance) { sInstance = Get(); }

		if (!sInstance->mRunning)
		{
			sInstance->mThreads.push_back(std::thread([&] { sInstance->ProcessAndSetDataThread(); }));
			sInstance->mThreads.back().detach();
			sInstance->mRunning = true;
		}
	}

	void ProcessManager::Stop()
	{
		if (sInstance && sInstance->mRunning)
		{
			sInstance->mRunning = false;
			std::thread kill([&]() { sInstance->Terminate(); });
			kill.detach();
		}
	}

	void ProcessManager::ProcessAndSetDataThread()
	{
		while (mRunning)
		{
			auto data = GetProcessData();
			SetData(data);
			Sleep(1000);
		}
	}

	ProcessArray* ProcessManager::GetProcessData()
	{
		HANDLE hProcessSnap;
		ProcessArray* data = nullptr;
		static PROCESSENTRY32 processEntry{};
		processEntry.dwSize = sizeof(PROCESSENTRY32);

		// Take a snapshot of all processes in the system.
		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			PrintWin32Error("CreateToolhelp32Snapshot (of processes)");
		}

		if (!Process32First(hProcessSnap, &processEntry)) {
			PrintWin32Error("Process32First");
			CloseHandle(hProcessSnap);          // clean the snapshot object
		}

		data = new ProcessArray();

		// Now walk the snapshot of processes, and
		// get information about each process in turn
		do {
			data->Entries.emplace_back(new ProcessEntry(processEntry));

			/* TODO: Implement these features
			// List the modules and threads associated with this process
			ListProcessModules(procEntry.th32ProcessID);
			ListProcessThreads(procEntry.th32ProcessID);
			*/

		} while (Process32Next(hProcessSnap, &processEntry));

		CloseHandle(hProcessSnap);

		return data;
	}

	void ProcessManager::SetData(ProcessArray* data)
	{
		if (!data) { return; }

		std::mutex mutex;
		std::unique_lock<std::mutex> lock(mutex);

		auto& lc = GetLockContainer();
		auto& writeLock = lc.GetWriteLock();
		auto& readLock = lc.GetReadLock();

		while (writeLock.owns_lock()) { lc.Wait(lock); }

		writeLock.lock();
		lc.Wait(writeLock, !readLock.owns_lock()); // Don't set the data while it's being read!

		mDataReady = false;
		lc.NotifyAll(); // Notify all that the state has changed

		mProcessEntries->Destroy();
		mProcessEntries.reset(data);
		mDataReady = true;

		writeLock.unlock();
		lc.NotifyAll();
	}

}