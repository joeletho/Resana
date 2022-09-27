#pragma once

#include <TlHelp32.h>
#include <system/cpu/LogicalCoreData.h>

namespace RESANA {

class ProcessEntry;

class Process {
public:
  Process();
  Process(const PROCESSENTRY32 &pe32);
  Process(const Process *process);
  ~Process();

  Process &operator=(const PROCESSENTRY32 &pe32);
  Process &operator=(const Process *process);

protected:
  std::string mName;
  uint32_t mId{};
  uint32_t mParentId{};
  uint32_t mModuleId{};
  uint32_t mThreadCount{};
  uint32_t mPriorityClass{};
  uint32_t mFlags{};
  uint64_t mPrivateUsage{};
  uint64_t mWorkingSetSize{};
  std::atomic<double> mCpuLoad{0};
  std::shared_ptr<PdhData> mData = nullptr;
  bool mRunning = true;

private:
  inline static uint32_t sDefaultId = 0;
};

} // namespace RESANA