#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include <Pdh.h>
#include <tchar.h>

namespace RESANA {

typedef PDH_FMT_COUNTERVALUE_ITEM PdhItem;

struct PdhData {
  HANDLE Handle{};
  HQUERY Query{};
  PDH_HCOUNTER Counter{};
  uint64_t Time{};
  uint64_t SystemTime{};
  uint64_t UserTime{};
  uint64_t CreationTime{};
  uint64_t ExitTime{};

  PdhData &operator=(const PdhData *data) {
    if (Handle) {
      CloseHandle(Handle);
    }
    if (Query) {
      PdhCloseQuery(Query);
    }
    if (Counter) {
      PdhRemoveCounter(Counter);
    }
    Handle = data->Handle;
    Query = data->Query;
    Counter = data->Counter;

    ZeroMemory(&Time, sizeof(ULARGE_INTEGER));
    memcpy(&Time, &data->Time, sizeof(ULARGE_INTEGER));

    ZeroMemory(&UserTime, sizeof(ULARGE_INTEGER));
    memcpy(&UserTime, &data->UserTime, sizeof(ULARGE_INTEGER));

    ZeroMemory(&CreationTime, sizeof(ULARGE_INTEGER));
    memcpy(&CreationTime, &data->CreationTime, sizeof(ULARGE_INTEGER));

    ZeroMemory(&SystemTime, sizeof(ULARGE_INTEGER));
    memcpy(&SystemTime, &data->SystemTime, sizeof(ULARGE_INTEGER));

    ZeroMemory(&ExitTime, sizeof(ULARGE_INTEGER));
    memcpy(&ExitTime, &data->ExitTime, sizeof(ULARGE_INTEGER));

    return *this;
  }
};

class LogicalCoreData {
public:
    LogicalCoreData();
    LogicalCoreData(const LogicalCoreData& other);
    ~LogicalCoreData();

    std::mutex& GetMutex();

    std::vector<std::shared_ptr<PdhItem>>& GetProcessors();

    void SetProcessorRef(PdhItem* ref);
    [[nodiscard]] PdhItem* GetProcessorRef() const;

    [[nodiscard]] DWORD& GetSize();

    DWORD& GetBuffer();

    void Clear();
    void Copy(const LogicalCoreData& other);

    LogicalCoreData& operator=(const LogicalCoreData& rhs);

private:
    std::mutex mMutex {};
    std::vector<std::shared_ptr<PdhItem>> mProcessors {};
    PdhItem* mProcessorRef = nullptr;
    DWORD mSize = 0;
    DWORD mBuffer = 0;
};

}
