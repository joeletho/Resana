#pragma once

#include <mutex>
#include <vector>

#include "ProcessEntry.h"

namespace RESANA {
class ProcessEntry;

class ProcessContainer {
public:
  ProcessContainer();
  ProcessContainer(const ProcessContainer &other);
  ~ProcessContainer();

  std::mutex &GetMutex();
  [[nodiscard]] int GetNumEntries() const;

  std::vector<std::shared_ptr<ProcessEntry>> &GetEntries();
  [[nodiscard]] std::shared_ptr<ProcessEntry> GetSelectedEntry() const;
  [[nodiscard]] std::shared_ptr<ProcessEntry>
  FindEntry(const std::shared_ptr<ProcessEntry> &entry) const;
  [[nodiscard]] std::shared_ptr<ProcessEntry> FindEntry(uint32_t procId) const;

  void AddEntry(std::shared_ptr<ProcessEntry> &entry);
  void SelectEntry(uint32_t procId, bool preserve = false);
  void SelectEntry(std::shared_ptr<ProcessEntry> &entry, bool preserve = false);
  void EraseEntry(std::shared_ptr<ProcessEntry> &entry);
  void Copy(ProcessContainer &other);

  void UpdateEntries();
  void SetClean() { mDirty = false; }
  void SetDirty() { mDirty = true; }
  bool IsDirty() const { return mDirty; }

  template <class _MutTy> void Clear(_MutTy &mutex);

  bool Contains(uint32_t procId) const;

  ProcessContainer &operator=(const ProcessContainer &other);

private:
  std::vector<std::shared_ptr<ProcessEntry>> mEntries{};
  std::mutex mMutex{};
  bool mDirty = false;

  std::shared_ptr<ProcessEntry> mSelectedEntry = nullptr;
};

template <class _MutTy> void ProcessContainer::Clear(_MutTy &mutex) {
  std::scoped_lock slock(mutex);
  for (auto &entry : mEntries) {
    entry.reset();
    entry = nullptr;
  }

  mEntries.clear();
}

} // namespace RESANA
