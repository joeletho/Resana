#pragma once

#include "LogicalCoreData.h"
#include "system/base/SystemObject.h"

#include "helpers/Time.h"

#include <deque>
#include <memory>
#include <queue>

#include "system/processes/Process.h"

namespace RESANA {

struct PdhData;

class CpuPerformance : public SystemObject {
public:
  ~CpuPerformance() override;
  CpuPerformance(const CpuPerformance &other);

  void Run() override;
  void Stop() override;
  void Shutdown() override;

  static std::shared_ptr<CpuPerformance> Get();

  std::shared_ptr<LogicalCoreData> GetData();

  [[nodiscard]] int GetNumProcessors() const;
  [[nodiscard]] static double GetCurrentLoad();
  [[nodiscard]] static double GetCurrentProcessLoad();
  [[nodiscard]] static double GetProcessLoad(uint32_t procId, PdhData *data);
  static void GetProcessTimes(PdhData &data);
  float GetCpuLoad();

  // Must be called after GetData() to unlock mutex
  void ReleaseData();
  void SetUpdateInterval(Timestep interval);

  bool IsRunning() const;

private:
  CpuPerformance();

  // Initializer
  void InitCpuData();
  void InitProcessData();

  // Threads
  void PrepareDataThread();
  void ProcessDataThread();

  // Called from threads
  [[nodiscard]] std::shared_ptr<LogicalCoreData> PrepareData() const;
  std::shared_ptr<LogicalCoreData> ExtractData();
  void SetData(std::shared_ptr<LogicalCoreData> &data);
  void PushData(const std::shared_ptr<LogicalCoreData> &data);
  void ProcessData(std::shared_ptr<LogicalCoreData> &data);
  static float CalcCpuLoad(uint64_t idleTicks, uint64_t totalTicks);
  static double CalcProcessLoad(const uint32_t procId, PdhData *data);

  static std::shared_ptr<LogicalCoreData>
  SortAscending(std::shared_ptr<LogicalCoreData> &data);

private:
  const unsigned int MAX_LOAD_COUNT = 3;

  bool mRunning = false;
  uint32_t mUpdateInterval{};
  std::atomic<bool> mDataReady;
  std::atomic<bool> mDataBusy;

  std::shared_ptr<LogicalCoreData> mLogicalCoreData{};
  std::queue<std::shared_ptr<LogicalCoreData>> mDataQueue{};
  std::deque<double> mCpuLoadValues{};

  double mCpuLoadAvg{};
  double mProcessLoad{};
  int mNumProcessors{};

  PdhData mCpuData;
  PdhData mLoadData;
  PdhData mProcData;

  static std::shared_ptr<CpuPerformance> sInstance;
};
} // namespace RESANA
