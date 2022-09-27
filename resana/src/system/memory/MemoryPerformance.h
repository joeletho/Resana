#pragma once

#include "helpers/Time.h"
#include <system/base/SystemObject.h>

#include <Windows.h>
#include <memory>

#include <Psapi.h>

namespace RESANA {

constexpr auto BYTES_PER_KB = 1024;
constexpr auto BYTES_PER_MB = 1048576;

class MemoryPerformance : public SystemObject {
public:
  ~MemoryPerformance();

  static std::shared_ptr<MemoryPerformance> Get();

  void Run() override;
  void Stop() override;
  void Shutdown() override;

  static ULONG64 GetPrivateUsage(uint32_t procId);
  static ULONG64 GetPrivateUsageKB(uint32_t procId);
  static ULONG64 GetPrivateUsageMB(uint32_t procId);
  static ULONG64 GetWorkingSetSize(uint32_t procId);
  static ULONG64 GetWorkingSetSizeKB(uint32_t procId);
  static ULONG64 GetWorkingSetSizeMB(uint32_t procId);

  static DWORD GetMemoryLoad(int);
  [[nodiscard]] float GetMemoryLoad() const;

  /* Physical Memory */
  [[nodiscard]] DWORDLONG GetTotalPhysical() const;
  [[nodiscard]] DWORDLONG GetTotalPhysicalKB() const;
  [[nodiscard]] DWORDLONG GetTotalPhysicalMB() const;
  [[nodiscard]] DWORDLONG GetAvailPhysical() const;
  [[nodiscard]] DWORDLONG GetAvailPhysicalKB() const;
  [[nodiscard]] DWORDLONG GetAvailPhysicalMB() const;
  [[nodiscard]] DWORDLONG GetUsedPhysical() const;
  [[nodiscard]] DWORDLONG GetUsedPhysicalKB() const;
  [[nodiscard]] DWORDLONG GetUsedPhysicalMB() const;

  /* Virtual Memory */
  [[nodiscard]] DWORDLONG GetTotalVirtual() const;
  [[nodiscard]] DWORDLONG GetAvailVirtual() const;
  [[nodiscard]] DWORDLONG GetUsedVirtual() const;
  [[nodiscard]] SIZE_T GetCurrProcUsageVirtual() const;

  [[nodiscard]] bool IsRunning() const { return mRunning; }

  void SetUpdateInterval(Timestep interval = TimeTick::Rate::Normal);

private:
  MemoryPerformance();
  void UpdateMemoryInfo();
  void UpdatePmc();

  static void GetProcessMemoryInformation(PROCESS_MEMORY_COUNTERS_EX &dest,
                                          uint32_t procId);

private:
  MEMORYSTATUSEX mMemoryInfo{};
  PROCESS_MEMORY_COUNTERS_EX mPmc{};
  uint32_t mUpdateInterval{};
  bool mRunning = false;

  static std::shared_ptr<MemoryPerformance> sInstance;
};

#define GET_PRIVATE_USAGE MemoryPerformance::GetPrivateUsage
#define GET_PRIVATE_USAGE_KB MemoryPerformance::GetPrivateUsedK
#define GET_PRIVATE_USAGE_MB MemoryPerformance::GetPrivateUsedMB
#define GET_WORKING_SET MemoryPerformance::GetWorkingSetSize
#define GET_WORKING_SET_KB MemoryPerformance::GetWorkingSetSizeKB
#define GET_WORKING_SET_MB MemoryPerformance::GetWorkingSetSizeMB

} // namespace RESANA
