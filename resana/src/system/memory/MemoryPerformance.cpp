#include "MemoryPerformance.h"
#include "rspch.h"

#include "core/Application.h"
#include "core/Core.h"

namespace RESANA {

std::shared_ptr<MemoryPerformance> MemoryPerformance::sInstance = nullptr;

MemoryPerformance::MemoryPerformance()
    : SystemObject(this), mUpdateInterval(TimeTick::Rate::Normal) {}

MemoryPerformance::~MemoryPerformance() {
  // Wait for loops to stop
  Time::Sleep(mUpdateInterval);
  RS_CORE_TRACE("MemoryPerformance destroyed");
}

std::shared_ptr<MemoryPerformance> MemoryPerformance::Get() {
  if (!sInstance) {
    sInstance.reset(new MemoryPerformance());
  }

  return sInstance;
}

void MemoryPerformance::Run() {
  if (!sInstance) {
    sInstance = Get();
  }

  if (!IsRunning()) {
    mRunning = true;

    const auto &app = Application::Get();
    auto &threadPool = app.GetThreadPool();

    threadPool.Queue([&] { sInstance->UpdateMemoryInfo(); });
    threadPool.Queue([&] { sInstance->UpdatePmc(); });
  }
}

void MemoryPerformance::Stop() {
  if (sInstance && IsRunning()) {
    mRunning = false;
  }
}

void MemoryPerformance::Shutdown() {
  if (sInstance) {
    Stop();
    sInstance.reset();
  }
}

float MemoryPerformance::GetMemoryLoad() const {
  if (const auto divisor = (float)GetTotalPhysical(); divisor != 0.0f) {
    return (float)GetUsedPhysical() / divisor * 100.0f;
  }
  return 0;
}
DWORD MemoryPerformance::GetMemoryLoad(int) {
  MEMORYSTATUSEX memInfo{};
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);

  GlobalMemoryStatusEx(&memInfo);
  const auto result = memInfo.dwMemoryLoad;
  ZeroMemory(&memInfo, sizeof(MEMORYSTATUSEX));

  return result;
}

DWORDLONG MemoryPerformance::GetTotalPhysical() const {
  return mMemoryInfo.ullTotalPhys;
}

DWORDLONG MemoryPerformance::GetTotalPhysicalKB() const {
  return GetTotalPhysical() / BYTES_PER_KB;
}

DWORDLONG MemoryPerformance::GetTotalPhysicalMB() const {
  return GetTotalPhysical() / BYTES_PER_MB;
}

DWORDLONG MemoryPerformance::GetAvailPhysical() const {
  return mMemoryInfo.ullAvailPhys;
}

DWORDLONG MemoryPerformance::GetAvailPhysicalKB() const {
  return GetAvailPhysical() / BYTES_PER_KB;
}

DWORDLONG MemoryPerformance::GetAvailPhysicalMB() const {
  return GetAvailPhysical() / BYTES_PER_MB;
}

DWORDLONG MemoryPerformance::GetUsedPhysical() const {
  return mMemoryInfo.ullTotalPhys - mMemoryInfo.ullAvailPhys;
}

DWORDLONG MemoryPerformance::GetUsedPhysicalKB() const {
  return GetUsedPhysical() / BYTES_PER_KB;
}

DWORDLONG MemoryPerformance::GetUsedPhysicalMB() const {
  return GetUsedPhysical() / BYTES_PER_MB;
}

void MemoryPerformance::GetProcessMemoryInformation(
    PROCESS_MEMORY_COUNTERS_EX &dest, uint32_t procId) {
  HANDLE hProcess{};

  if ((hProcess =
           OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, procId))) {
    // GetProcessInformation(hProcess, ProcessAppMemoryInfo, &dest,
    // sizeof(APP_MEMORY_INFORMATION));
    ::GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&dest,
                           sizeof(PROCESS_MEMORY_COUNTERS_EX));
  } else {
    //::ZeroMemory(&dest, sizeof(APP_MEMORY_INFORMATION));
    ::ZeroMemory(&dest, sizeof(PROCESS_MEMORY_COUNTERS_EX));
  }
  CloseHandle(hProcess);
}

ULONG64 MemoryPerformance::GetPrivateUsage(uint32_t procId) {
  static PROCESS_MEMORY_COUNTERS_EX info{};
  ZeroMemory(&info, sizeof(PROCESS_MEMORY_COUNTERS_EX));
  GetProcessMemoryInformation(info, procId);

  return info.PrivateUsage;
}

ULONG64 MemoryPerformance::GetPrivateUsageKB(uint32_t procId) {
  return GetPrivateUsage(procId) / BYTES_PER_KB;
}

ULONG64 MemoryPerformance::GetPrivateUsageMB(uint32_t procId) {
  return GetPrivateUsage(procId) / BYTES_PER_MB;
}

ULONG64 MemoryPerformance::GetWorkingSetSize(uint32_t procId) {
  static PROCESS_MEMORY_COUNTERS_EX info{};
  ZeroMemory(&info, sizeof(PROCESS_MEMORY_COUNTERS_EX));
  GetProcessMemoryInformation(info, procId);

  return info.WorkingSetSize;
}

ULONG64 MemoryPerformance::GetWorkingSetSizeKB(uint32_t procId) {
  return GetWorkingSetSize(procId) / BYTES_PER_KB;
}

ULONG64 MemoryPerformance::GetWorkingSetSizeMB(uint32_t procId) {
  return GetWorkingSetSize(procId) / BYTES_PER_MB;
}

DWORDLONG MemoryPerformance::GetTotalVirtual() const {
  return mMemoryInfo.ullTotalPageFile;
}

DWORDLONG MemoryPerformance::GetAvailVirtual() const {
  return mMemoryInfo.ullAvailVirtual;
}

DWORDLONG MemoryPerformance::GetUsedVirtual() const {
  return mMemoryInfo.ullTotalPageFile - mMemoryInfo.ullAvailPageFile;
}

SIZE_T MemoryPerformance::GetCurrProcUsageVirtual() const {
  return mPmc.PrivateUsage;
}

void MemoryPerformance::SetUpdateInterval(const Timestep interval) {
  mUpdateInterval = (uint32_t)interval;
}

void MemoryPerformance::UpdateMemoryInfo() {
  MEMORYSTATUSEX memInfo{};
  mMemoryInfo = memInfo;
  mMemoryInfo.dwLength = sizeof(MEMORYSTATUSEX);

  do {
    GlobalMemoryStatusEx(&mMemoryInfo);
  } while (IsRunning() && Time::Sleep(mUpdateInterval));
  ZeroMemory(&mMemoryInfo, sizeof(MEMORYSTATUSEX));
}

void MemoryPerformance::UpdatePmc() {
  PROCESS_MEMORY_COUNTERS_EX pmc{};
  mPmc = pmc;

  const auto hProcess = GetCurrentProcess();
  do {
    ZeroMemory(&mPmc, sizeof(PROCESS_MEMORY_COUNTERS_EX));
    GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS *)&mPmc,
                         sizeof(mPmc));
  } while (IsRunning() && Time::Sleep(mUpdateInterval));

  CloseHandle(hProcess);
  ZeroMemory(&mPmc, sizeof(PROCESS_MEMORY_COUNTERS_EX));
}

} // namespace RESANA
