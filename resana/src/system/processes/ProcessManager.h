#pragma once

#include "system/base/SystemObject.h"

#include "ProcessContainer.h"
#include "ProcessEntry.h"
#include "ProcessMap.h"

#include "helpers/Time.h"

#include <memory>

namespace RESANA {

class ProcessManager final : public SystemObject {
public:
  ~ProcessManager() override;

  static std::shared_ptr<ProcessManager> Get();

  void Run() override;
  void Stop() override;
  void Shutdown() override;

  [[nodiscard]] int GetNumProcesses();

  static void SyncProcessContainer(ProcessContainer &container);

  void SetUpdateInterval(Timestep interval = TimeTick::Rate::Normal);
  uint32_t GetUpdateSpeed() const;

  bool IsRunning() const;

private:
  ProcessManager();

  static void Destroy();
  bool ShouldClose() const;
  void PrepareDataThread();

  bool PrepareData();
  void GetPreparedData(ProcessContainer &container);

  bool UpdateProcess(int procId);

  void CleanMap();
  void ResetAllRunningStatus();

private:
  ProcessMap mProcessMap{};
  bool mRunning = false;
  uint32_t mUpdateInterval{};
  std::atomic<bool> mDataPrepared;
  std::atomic<bool> mDataReady;
  std::atomic<bool> mDataBusy;

  static std::shared_ptr<ProcessManager> sInstance;
};

} // namespace RESANA
