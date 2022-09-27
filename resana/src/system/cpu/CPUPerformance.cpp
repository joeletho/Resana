#include "CpuPerformance.h"
#include "rspch.h"

#include "core/Application.h"
#include "core/Core.h"

#include "helpers/WinFuncs.h"

#include <memory>
#include <mutex>

#include <PdhMsg.h>
#include <Windows.h>

#include "system/processes/Process.h"

namespace RESANA {

std::shared_ptr<CpuPerformance> CpuPerformance::sInstance = nullptr;

CpuPerformance::CpuPerformance()
    : SystemObject(this), mUpdateInterval(TimeTick::Rate::Normal) {
  mLogicalCoreData = std::make_shared<LogicalCoreData>();
}

CpuPerformance::CpuPerformance(const CpuPerformance &other)
    : SystemObject(other.mContext) {
  mRunning = other.mRunning;
  mUpdateInterval = other.mUpdateInterval;
  mDataReady.store(other.mDataReady);
  mDataBusy.store(other.mDataBusy);
  mLogicalCoreData.reset(other.mLogicalCoreData.get());
  mDataQueue = other.mDataQueue;
  mCpuLoadValues = other.mCpuLoadValues;
  mCpuLoadAvg = other.mCpuLoadAvg;
  mProcessLoad = other.mProcessLoad;
  mNumProcessors = other.mNumProcessors;
  mCpuData = other.mCpuData;
  mLoadData = other.mLoadData;
  mProcData = other.mProcData;
}

CpuPerformance::~CpuPerformance() {
  Time::Sleep(mUpdateInterval); // Let threads finish before destructing

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (mDataBusy) {
    mLockContainer.Wait(lock);
  }
  RS_CORE_TRACE("CpuPerformance destroyed");
}

void CpuPerformance::InitCpuData() {
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  mNumProcessors = (int)sysInfo.dwNumberOfProcessors;

  PDH_STATUS pdhStatus = PdhOpenQuery(nullptr, 0, &mCpuData.Query);

  if (pdhStatus != ERROR_SUCCESS) {
    RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
  }

  // Specify a counter object with a wildcard for the instance.
  pdhStatus =
      PdhAddCounter(mCpuData.Query, TEXT("\\Processor(*)\\% Processor Time"), 0,
                    &mCpuData.Counter);
  if (pdhStatus != ERROR_SUCCESS) {
    RS_CORE_ERROR("PdhAddData failed with 0x{0}", pdhStatus);
  }
}

void CpuPerformance::InitProcessData() {
  mProcData.Handle = GetCurrentProcess();
  GetProcessTimes(mProcData);
  ::CloseHandle(mProcData.Handle);
}

void CpuPerformance::Run() {
  if (!sInstance) {
    sInstance = Get();
  }

  if (!sInstance->IsRunning()) {
    sInstance->mRunning = true;

    const auto &app = Application::Get();
    auto &threadPool = app.GetThreadPool();

    threadPool.Queue([&] { sInstance->PrepareDataThread(); });
    threadPool.Queue([&] { sInstance->ProcessDataThread(); });
  }
}

void CpuPerformance::Stop() {
  if (IsRunning()) {
    mRunning = false;
    mLockContainer.NotifyAll();
  }
}

void CpuPerformance::Shutdown() {
  Stop();
  const auto &app = Application::Get();
  sInstance.reset();
}

std::shared_ptr<CpuPerformance> CpuPerformance::Get() {
  if (!sInstance) {
    sInstance.reset(new CpuPerformance);
    sInstance->InitCpuData();
    sInstance->InitProcessData();
  }

  return sInstance;
}

std::shared_ptr<LogicalCoreData> CpuPerformance::GetData() {
  RS_CORE_ASSERT(IsRunning(), "Process is not currently running! Call "
                              "'CpuPerformance::Run()' to start process.");

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  if (!mDataReady) {
    // We wait a little bit to make the UI more streamlined -- otherwise, we may
    // see some random flickering.
    mLockContainer.WaitFor(lock, 10, mDataReady);
  }
  if (!mDataReady) {
    return {};
  } // If data still isn't ready, return nothing.

  mDataBusy = true;
  mLockContainer.NotifyAll();

  return mLogicalCoreData;
}

int CpuPerformance::GetNumProcessors() const { return mNumProcessors; }

double CpuPerformance::GetCurrentLoad() {
  if (sInstance) {
    return sInstance->mCpuLoadAvg;
  }
  return 0.0;
}

double CpuPerformance::GetProcessLoad(const uint32_t procId, PdhData *data) {
  return CalcProcessLoad(procId, data);
}

double CpuPerformance::GetCurrentProcessLoad() {
  return CalcProcessLoad(GetCurrentProcessId(), &sInstance->mProcData);
}

void CpuPerformance::ReleaseData() {
  mDataBusy = false;

  mLockContainer.NotifyAll();
}

void CpuPerformance::SetUpdateInterval(Timestep interval) {
  mUpdateInterval = interval;
}

bool CpuPerformance::IsRunning() const { return mRunning; }

void CpuPerformance::PrepareDataThread() {
  while (IsRunning()) {
    auto data = PrepareData();
    PushData(data);
  }
}

void CpuPerformance::ProcessDataThread() {
  while (IsRunning()) {
    auto data = ExtractData();
    ProcessData(data);
    SetData(data);
  }
}

std::shared_ptr<LogicalCoreData> CpuPerformance::PrepareData() const {
  auto data = new LogicalCoreData();
  bool success = true;
  PdhItem *processorRef = nullptr;

  // Some counters need two samples in order to format a value, so
  // make this call to get the first value before entering the loop.
  PDH_STATUS pdhStatus = PdhCollectQueryData(mCpuData.Query);

  if (pdhStatus != ERROR_SUCCESS) {
    RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
    success = false;
  }

  Time::Sleep(mUpdateInterval); // Sleep for 1 second on this thread.

  pdhStatus = PdhCollectQueryData(mCpuData.Query);
  if (pdhStatus != ERROR_SUCCESS) {
    RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
    success = false;
  }

  // Get the required size of the data buffer.
  pdhStatus = PdhGetFormattedCounterArray(mCpuData.Counter, PDH_FMT_DOUBLE,
                                          &data->GetBuffer(), &data->GetSize(),
                                          processorRef);

  if (pdhStatus != (long)PDH_MORE_DATA) {
    RS_CORE_ERROR("PdhGetFormattedDataArray failed with 0x{0}", pdhStatus);
    success = false;
  }

  processorRef = (PdhItem *)malloc(data->GetBuffer());
  if (!processorRef) {
    RS_CORE_ERROR("malloc for PdhGetFormattedDataArray failed 0x{0}",
                  pdhStatus);
    success = false;
  }

  pdhStatus = PdhGetFormattedCounterArray(mCpuData.Counter, PDH_FMT_DOUBLE,
                                          &data->GetBuffer(), &data->GetSize(),
                                          processorRef);

  if (pdhStatus != ERROR_SUCCESS) {
    RS_CORE_ERROR("PdhGetFormattedDataArray failed with 0x{0}", pdhStatus);
    success = false;
  }

  if (success) {
    data->SetProcessorRef(processorRef);
  } else {
    // In case of an error, free the memory
    delete data;
    data = nullptr;
    return nullptr;
  }

  return std::make_shared<LogicalCoreData>(*data);
}

void CpuPerformance::PushData(const std::shared_ptr<LogicalCoreData> &data) {
  if (!data) {
    return;
  }

  std::mutex mutex;
  std::lock(mutex, mLockContainer.GetMutex());
  {
    std::lock_guard lock1(mutex, std::adopt_lock);
    std::lock_guard lock2(mLockContainer.GetMutex(), std::adopt_lock);
    mDataQueue.push(data);
  }
  mLockContainer.NotifyAll();
}

std::shared_ptr<LogicalCoreData> CpuPerformance::ExtractData() {
  std::shared_ptr<LogicalCoreData> data = nullptr;
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (mDataQueue.empty()) {
    if (!IsRunning()) {
      return nullptr;
    }
    mLockContainer.Wait(lock);
  }
  lock.unlock();
  std::lock(mutex, mLockContainer.GetMutex());
  {
    std::lock_guard lock1(mutex, std::adopt_lock);
    std::lock_guard lock2(mLockContainer.GetMutex(), std::adopt_lock);

    data = mDataQueue.front();
    mDataQueue.pop();
  }

  mLockContainer.NotifyAll();

  return data;
}

void CpuPerformance::ProcessData(std::shared_ptr<LogicalCoreData> &data) {
  if (!data) {
    return;
  }

  std::mutex mutex;

  const auto processorPtr = data->GetProcessorRef();

  // Loop through the array and add _Total to deque and cpu values into the
  // local vector
  for (DWORD i = 0; i < data->GetSize(); ++i) {
    const auto processor = new PdhItem(processorPtr[i]);

    const auto name = processor->szName;
    auto value = processor->FmtValue.doubleValue;

    if (std::strcmp(name, "_Total") == 0) {
      std::lock(mutex, mLockContainer.GetMutex());
      std::lock_guard lock1(mutex, std::adopt_lock);
      std::lock_guard lock2(mLockContainer.GetMutex(), std::adopt_lock);

      // Remove the oldest value
      if (mCpuLoadValues.size() == MAX_LOAD_COUNT) {
        mCpuLoadValues.pop_front();
      }

      // Add the current value and compute the average
      mCpuLoadValues.push_back(value);
      mCpuLoadAvg = CalculateAverage(mCpuLoadValues);
    } else {
      // Add the processor
      auto copy = std::make_shared<PdhItem>(*processor);
      data->GetProcessors().emplace_back(copy);
    }
  }
}

float CpuPerformance::GetCpuLoad() {
  FILETIME idleTime, kernelTime, userTime;
  auto loadNorm =
      GetSystemTimes(&idleTime, &kernelTime, &userTime)
          ? CalcCpuLoad(FileTimeToInt64(idleTime),
                        FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime))
          : -1.0f;
  auto loadPerc = loadNorm * 100.0f;
  return loadPerc;
}

float CpuPerformance::CalcCpuLoad(uint64_t idleTicks, uint64_t totalTicks) {
  static uint64_t previousTotalTicks = 0;
  static uint64_t previousIdleTicks = 0;

  uint64_t totalTicksSinceLastTime = totalTicks - previousTotalTicks;
  uint64_t idleTicksSinceLastTime = idleTicks - previousIdleTicks;

  float ret =
      1.0f - ((totalTicksSinceLastTime > 0)
                  ? ((float)idleTicksSinceLastTime) / totalTicksSinceLastTime
                  : 0);

  previousTotalTicks = totalTicks;
  previousIdleTicks = idleTicks;
  return ret;
}

double CpuPerformance::CalcProcessLoad(const uint32_t procId, PdhData *data) {
  static SYSTEM_INFO sysInfo{};
  ::GetSystemInfo(&sysInfo);
  static auto cpuCount = (int)sysInfo.dwNumberOfProcessors;

  HANDLE handle;
  if (procId == ::GetCurrentProcessId()) {
    handle = ::GetCurrentProcess();
  } else {
    handle = ::OpenProcess(MAXIMUM_ALLOWED, FALSE, procId);
  }

  PdhData times{};
  times.Handle = handle;
  GetProcessTimes(times);
//   CloseHandle(handle);

  const auto now = times.Time;
  const auto last = data->Time;
  const auto total =
      (times.SystemTime - data->SystemTime) + (times.UserTime - data->UserTime);
  double elapsed = 1.0;
  if (const auto diff = (double)(now - last); diff != 0.0) {
    elapsed = diff;
  }

  data->Time = times.Time;
  data->UserTime = times.UserTime;
  data->SystemTime = times.SystemTime;
  data->CreationTime = times.CreationTime;

  return (double)total / elapsed / cpuCount * 100.0;
}

void CpuPerformance::SetData(std::shared_ptr<LogicalCoreData> &data) {
  if (!sInstance || !data) {
    return;
  }

  SortAscending(data);

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (mDataBusy) {
    if (!IsRunning()) {
      return;
    }
    mLockContainer.Wait(lock);
  }
  lock.unlock();

  mDataReady = false;
  std::lock(mutex, mLockContainer.GetMutex());
  {
    std::lock_guard lock1(mutex, std::adopt_lock);
    std::lock_guard lock2(mLockContainer.GetMutex(), std::adopt_lock);

    mLogicalCoreData = data;
  }
  mDataReady = true;
  mLockContainer.NotifyAll();
}

std::shared_ptr<LogicalCoreData>
CpuPerformance::SortAscending(std::shared_ptr<LogicalCoreData> &data) {
  auto &processors = data->GetProcessors();
  std::sort(processors.begin(), processors.end(),
            [&](const std::shared_ptr<PdhItem> &left,
                const std::shared_ptr<PdhItem> &right) {
              return std::stoi(left->szName) < std::stoi(right->szName);
            });

  return data;
}

void CpuPerformance::GetProcessTimes(PdhData &data) {
  FILETIME currentTime{}, systemTime{}, userTime{}, creationTime{}, exitTime{};
  GetSystemTimeAsFileTime(&currentTime);
  ::GetProcessTimes(data.Handle, &creationTime, &exitTime, &systemTime,
                    &userTime);

  data.Time = FileTimeToInt64(currentTime);
  data.SystemTime = FileTimeToInt64(systemTime);
  data.UserTime = FileTimeToInt64(userTime);
  data.CreationTime = FileTimeToInt64(creationTime);
  data.ExitTime = FileTimeToInt64(exitTime);
}

} // namespace RESANA