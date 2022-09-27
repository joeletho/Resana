#pragma once

#include "ProcessEntry.h"

#include <map>
#include <mutex>

namespace RESANA {

class ProcessMap {
  typedef unsigned long ulong;

public:
  ProcessMap();
  ~ProcessMap();

  std::recursive_mutex &GetMutex();

  [[nodiscard]] std::shared_ptr<ProcessEntry> Find(ulong procId);

  [[nodiscard]] int Count(ulong procId);
  [[nodiscard]] int Size() const;

  [[nodiscard]] bool Contains(ulong procId);
  [[nodiscard]] bool Empty() const;

  void Clear();
  void Emplace(std::shared_ptr<ProcessEntry> &entry);
  void Erase(ulong procId);
  void Erase(std::shared_ptr<ProcessEntry> &entry);

  // Overloads
  auto begin() { return mMap.begin(); }
  auto end() { return mMap.end(); }
  [[nodiscard]] auto cbegin() const { return mMap.cbegin(); }
  [[nodiscard]] auto cend() const { return mMap.cend(); }

  std::shared_ptr<ProcessEntry> operator[](ulong procId);

private:
  std::map<ulong, std::shared_ptr<ProcessEntry>> mMap{};
  std::recursive_mutex mMutex{};
  std::unique_lock<std::recursive_mutex> mLock;
};

} // namespace RESANA