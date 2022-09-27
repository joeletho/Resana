#include "ProcessManager.h"
#include "rspch.h"

#include "core/Core.h"

#include "helpers/FileUtils.h"
#include "helpers/WinFuncs.h"

#include <ProcessSnapshot.h>
#include <TlHelp32.h>
#include <Windows.h>

#include <memory>

#include "core/Application.h"

namespace RESANA {

std::shared_ptr<ProcessManager> ProcessManager::sInstance = nullptr;

ProcessManager::ProcessManager()
    : SystemObject(this), mUpdateInterval(TimeTick::Rate::Normal),
      mDataBusy(false) {}

ProcessManager::~ProcessManager() {
  Time::Sleep(mUpdateInterval); // Let detached threads finish before

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (mDataBusy) {
    std::this_thread::yield();
  }
  RS_CORE_TRACE("ProcessManager destroyed");
}

void ProcessManager::Destroy() { sInstance.reset(); }

std::shared_ptr<ProcessManager> ProcessManager::Get() {
  if (!sInstance) {
    sInstance.reset(new ProcessManager);
  }

  return sInstance;
}

void ProcessManager::SetUpdateInterval(Timestep interval) {
  mUpdateInterval = interval;
}

uint32_t ProcessManager::GetUpdateSpeed() const { return mUpdateInterval; }

bool ProcessManager::IsRunning() const {
  if (!sInstance) {
    return false;
  }
  return mRunning;
}

int ProcessManager::GetNumProcesses() { return mProcessMap.Size(); }

void ProcessManager::Run() {
  if (!sInstance) {
    sInstance = Get();
  }

  if (!IsRunning()) {
    mRunning = true;
    auto &app = Application::Get();
    auto &threadPool = app.GetThreadPool();
    threadPool.Queue([&] { PrepareDataThread(); });
  }
}

void ProcessManager::Stop() {
  if (sInstance && IsRunning()) {
    mRunning = false;
    mLockContainer.NotifyAll();
  }
}

void ProcessManager::Shutdown() {
  if (sInstance) {
    Stop();
    sInstance.reset();
    sInstance = nullptr;
  }
}

void ProcessManager::PrepareDataThread() {
  static std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex, std::defer_lock);

  while (!ShouldClose()) {
    if (PrepareData()) {
      mDataPrepared = true;
      mLockContainer.NotifyAll();
    }
    if (!lock.owns_lock()) {
      lock.lock();
    }
    mLockContainer.WaitFor(lock, mUpdateInterval);
  }
}

bool ProcessManager::PrepareData() {
  HANDLE hProcessSnap;
  PROCESSENTRY32 processEntry32{};
  processEntry32.dwSize = sizeof(PROCESSENTRY32);

  // Take a snapshot of all processes in the system.
  hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hProcessSnap == INVALID_HANDLE_VALUE) {
    PrintWin32Error("CreateToolhelp32Snapshot (of processes)");
    return false;
  }

  if (!Process32First(hProcessSnap, &processEntry32)) {
    PrintWin32Error("Process32First");
    CloseHandle(hProcessSnap); // clean the snapshot object
    return false;
  }

  ResetAllRunningStatus();

  // Now walk the snapshot of processes, and
  // get information about each process in turn
  do {
    if (ShouldClose()) {
      return false;
    }

    std::lock_guard lock2(mProcessMap.GetMutex());

    // Set process running status to true and update process, if applicable
    if (!UpdateProcess((int)processEntry32.th32ProcessID)) {
      // Otherwise, add new process
      auto processEntry = std::make_shared<ProcessEntry>(
          std::make_shared<Process>(processEntry32));
      mProcessMap.Emplace(processEntry);
    }
  } while (Process32Next(hProcessSnap, &processEntry32));

  CloseHandle(hProcessSnap);

  return true;
}

void ProcessManager::GetPreparedData(ProcessContainer &container) {
  if ((ShouldClose() || !mDataPrepared) && GetNumProcesses() == 0) {
    return;
  }

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  if (mDataBusy) {
    mLockContainer.Wait(lock, !mDataBusy);
  }
  lock.unlock();

  if (ShouldClose()) {
    return;
  }

  mDataPrepared = false;
  std::lock(container.GetMutex(), mProcessMap.GetMutex());
  {
    std::lock_guard lock1(container.GetMutex(), std::adopt_lock);
    std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

    for (auto &[id, entry] : mProcessMap) {
      if (auto existing = container.FindEntry(id)) {
        existing = entry;
      } else {
        auto copy = std::make_shared<ProcessEntry>(entry);
        container.AddEntry(copy);
      }
    }
  }

  mDataPrepared = true;
  mLockContainer.NotifyOne();
}

bool ProcessManager::UpdateProcess(int procId) {
  if (!mProcessMap.Contains(procId)) {
    return false;
  }
  if (auto proc = mProcessMap.Find(procId)) {
    proc->UpdatePerfStats();
    proc->mRunning = true;
  }
  return true;
}

void ProcessManager::CleanMap() {
  std::mutex mutex;
  std::lock(mutex, mProcessMap.GetMutex());
  {
    std::lock_guard lock(mutex, std::adopt_lock);
    std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

    // Remove any processes not currently running
    for (auto it = mProcessMap.begin(); it != mProcessMap.end(); ++it) {
      if (!it->second->IsRunning()) {
        mProcessMap.Erase(it->second);
        it = mProcessMap.begin(); // Reset iterator!
      }
    }
  }
}

void ProcessManager::ResetAllRunningStatus() {
  std::mutex mutex;
  std::lock(mutex, mProcessMap.GetMutex());
  {
    std::lock_guard lock(mutex, std::adopt_lock);
    std::lock_guard lock2(mProcessMap.GetMutex(), std::adopt_lock);

    for (const auto &[id, entry] : mProcessMap) {
      entry->mRunning = false;
    }
  }
}

void ProcessManager::SyncProcessContainer(ProcessContainer &container) {
  if (!sInstance) {
    return;
  }

  if (sInstance->GetNumProcesses() > 0) {
    sInstance->GetPreparedData(container);
    container.SetDirty();
  }
}
bool ProcessManager::ShouldClose() const { return !sInstance || !IsRunning(); }

} // namespace RESANA
