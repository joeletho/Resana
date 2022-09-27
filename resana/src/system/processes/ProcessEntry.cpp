#include "ProcessEntry.h"

#include <memory>

#include "system/cpu/CpuPerformance.h"
#include "system/memory/MemoryPerformance.h"

namespace RESANA {

ProcessEntry::ProcessEntry(const PROCESSENTRY32 &pe32)
    : Process(pe32), mLock(mMutex, std::defer_lock) {

}

ProcessEntry::ProcessEntry(const std::shared_ptr<Process> &process)
    : Process(process.get()), mLock(mMutex, std::defer_lock) {}

ProcessEntry::ProcessEntry(const std::shared_ptr<ProcessEntry> &entry)
    : Process(entry.get()), mLock(mMutex, std::defer_lock) {
  mRunning = entry->IsRunning();
}

ProcessEntry::~ProcessEntry() {
  // Acquire mutex and release upon destruction.
  std::scoped_lock slock(mMutex);
}

std::shared_ptr<PdhData> ProcessEntry::GetData() const { return this->mData; }

double ProcessEntry::GetCpuLoad() const { return this->mCpuLoad; }

ProcessEntry &ProcessEntry::operator=(ProcessEntry *entry) {
  if (entry) {
    std::scoped_lock slock(entry->mMutex);
    this->mName = entry->GetName();
    this->mId = entry->GetId();
    this->mParentId = entry->GetParentId();
    this->mModuleId = entry->GetModuleId();
    this->mThreadCount = entry->GetThreadCount();
    this->mPriorityClass = entry->GetPriorityClass();
    this->mFlags = entry->GetFlags();
    this->mCpuLoad = entry->GetCpuLoad();

    if (entry->GetData()) {
      this->mData = std::make_shared<PdhData>(*entry->GetData());
    }
    mSelected = entry->IsSelected();
    mRunning = entry->IsRunning();
  }

  return *this;
}

ProcessEntry &ProcessEntry::operator=(Process *other) {
  std::mutex mutex;
  std::lock(mutex, mMutex);
  {
    std::scoped_lock slock(std::adopt_lock, mutex, mMutex);
    auto *copy =
        new ProcessEntry(static_cast<const std::shared_ptr<Process>>(other));
    *this = copy;
  }
  return *this;
}

void ProcessEntry::SetData(std::shared_ptr<PdhData> &data) {
  std::scoped_lock slock(mMutex);
  if (!data) {
    this->mData.reset();
  } else {
    this->mData = std::make_shared<PdhData>(*data);
  }
}

void ProcessEntry::SetCpuLoad(float load) { this->mCpuLoad = load; }

void ProcessEntry::UpdatePerfStats() {
  std::scoped_lock slock(mMutex);
  this->mWorkingSetSize =
      (uint32_t)MemoryPerformance::GetPrivateUsage(this->mId);
  this->mPrivateUsage =
      (uint32_t)MemoryPerformance::GetWorkingSetSize(this->mId);
  this->mCpuLoad = CpuPerformance::GetProcessLoad(this->mId, this->mData.get());
}
} // namespace RESANA
