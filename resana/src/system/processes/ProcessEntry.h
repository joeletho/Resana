#pragma once

#include "Process.h"
#include <atomic>
#include <mutex>
#include <string>

namespace RESANA {

struct PdhData;

class ProcessEntry : public Process {
public:
  ProcessEntry(const PROCESSENTRY32 &pe32);
  ProcessEntry(const std::shared_ptr<Process> &process);
  ProcessEntry(const std::shared_ptr<ProcessEntry> &entry);
  ~ProcessEntry();

  [[nodiscard]] std::shared_ptr<PdhData> GetData() const;
  [[nodiscard]] double GetCpuLoad() const;

  std::string GetName() const { return mName; }
  uint32_t GetId() const { return mId; }
  uint32_t GetParentId() const { return mParentId; }
  uint32_t GetModuleId() const { return mModuleId; }
  uint32_t GetThreadCount() const { return mThreadCount; }
  uint32_t GetPriorityClass() const { return mPriorityClass; }
  uint32_t GetFlags() const { return mFlags; }
  uint64_t GetPrivateUsage() const { return mPrivateUsage; }
  uint64_t GetWorkingSetSize() const { return mWorkingSetSize; }
  std::shared_ptr<PdhData> GetData() { return this->mData; }
  bool IsRunning() const { return mRunning; }

  void SetName()  { mName; }
  void SetId(uint32_t id)  { mId = id; }
  void SetParentId(uint32_t id)  { mParentId = id; }
  void SetModuleId(uint32_t id)  { mModuleId = id; }
  void SetThreadCount(uint32_t count)  { mThreadCount = count; }
  void SetPriorityClass(uint32_t pclass)  { mPriorityClass = pclass; }
  void SetFlags(uint32_t  flags)  { mFlags = flags; }
  void SetPrivateUsage(uint32_t usage)  { mPrivateUsage = usage; }
  void SetWorkingSetSize(uint32_t size)  { mWorkingSetSize = size; }
  void SetData(std::shared_ptr<PdhData>& data);
  void SetCpuLoad(float load);

  void UpdatePerfStats();

  std::recursive_mutex &Mutex() { return mMutex; }
  [[nodiscard]] bool IsSelected() const { return mSelected; }

  // Overloads
  ProcessEntry &operator=(ProcessEntry *entry);
  ProcessEntry &operator=(Process *other);

private:
  void Select() { mSelected = true; }
  void Deselect() { mSelected = false; }

private:
  std::recursive_mutex mMutex{};
  std::unique_lock<std::recursive_mutex> mLock{};
  std::atomic<bool> mRunning{true};
  std::atomic<bool> mSelected{false};

  friend class ProcessManager;
  friend class ProcessContainer;
};

} // namespace RESANA
