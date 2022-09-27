#include "ProcessMap.h"
#include "rspch.h"

namespace RESANA {
ProcessMap::ProcessMap() : mLock(mMutex, std::defer_lock) {}

ProcessMap::~ProcessMap() {
  std::scoped_lock lock(mMutex);
  mMap.clear();
}

std::recursive_mutex &ProcessMap::GetMutex() { return mMutex; }

void ProcessMap::Emplace(std::shared_ptr<ProcessEntry> &entry) {
  mMap.try_emplace(entry->GetId(), entry);
}

void ProcessMap::Clear() {
  for (auto &[id, entry] : mMap) {
    Erase(entry);
  }
}

std::shared_ptr<ProcessEntry> ProcessMap::Find(const ulong procId) {
  std::shared_ptr<ProcessEntry> entry = nullptr;
  try {
    entry = mMap.at(procId);
  } catch (std::exception &e) {
    // Move along
  }

  return entry;
}

bool ProcessMap::Contains(const ulong procId) {
  if (const auto entry = Find(procId)) {
    return true;
  }

  return false;
}

bool ProcessMap::Empty() const { return mMap.empty(); }

int ProcessMap::Count(const ulong procId) {
  if (const auto proc = Find(procId)) {
    return (int)mMap.count(procId);
  }
  return 0;
}

int ProcessMap::Size() const { return (int)mMap.size(); }

void ProcessMap::Erase(const ulong procId) {
  if (auto proc = Find(procId)) {
    mMap.erase(procId);
    proc.reset();
    proc = nullptr;
  }
}

void ProcessMap::Erase(std::shared_ptr<ProcessEntry> &entry) {
  if (!entry) {
    return;
  }
  if (auto proc = Find(entry->GetId())) {
    if (proc != entry) // entry is a copy, delete both
    {
      entry.reset();
      entry = nullptr;
    }

    mMap.erase(proc->GetId());
    proc.reset();
    proc = nullptr;
  }
}

std::shared_ptr<ProcessEntry> ProcessMap::operator[](const ulong procId) {
  return Find(procId);
}

} // namespace RESANA
