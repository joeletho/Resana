#include "Process.h"

#include <memory>

namespace RESANA {

Process::Process() { mName = "Process " + std::to_string(sDefaultId++); }

Process::Process(const PROCESSENTRY32 &pe32) { *this = pe32; }

Process::Process(const Process *process) { *this = process; }

Process::~Process() = default;

Process &Process::operator=(const PROCESSENTRY32 &pe32) {
  mData = std::make_shared<PdhData>();
  mName = pe32.szExeFile;
  mId = pe32.th32ProcessID;
  mParentId = pe32.th32ParentProcessID;
  mModuleId = pe32.th32ModuleID;
  mThreadCount = pe32.cntThreads;
  mPriorityClass = pe32.pcPriClassBase;
  mFlags = pe32.dwFlags;
  return *this;
}

Process &Process::operator=(const Process *process) {
  if (!process) {
    mData.reset();
    mName = "Process " + std::to_string(sDefaultId++);
    mId = 0;
    mParentId = 0;
    mModuleId = 0;
    mThreadCount = 0;
    mPriorityClass = 0;
    mWorkingSetSize = 0;
    mPrivateUsage = 0;
    mFlags = 0;
    mCpuLoad = 0;
  } else {
    mData = std::make_shared<PdhData>(*process->mData);
    mName = process->mName;
    mId = process->mId;
    mParentId = process->mParentId;
    mModuleId = process->mModuleId;
    mThreadCount = process->mThreadCount;
    mPriorityClass = process->mPriorityClass;
    mWorkingSetSize = process->mWorkingSetSize;
    mPrivateUsage = process->mPrivateUsage;
    mFlags = process->mFlags;
    mCpuLoad = process->mCpuLoad._Storage._Value;
  }
  return *this;
}

} // namespace RESANA
