#include "ProcessContainer.h"
#include "rspch.h"

#include "ProcessEntry.h"

namespace RESANA {

ProcessContainer::ProcessContainer() = default;

ProcessContainer::ProcessContainer(const ProcessContainer &other)
    : mEntries(other.mEntries), mSelectedEntry(other.GetSelectedEntry()) {}

ProcessContainer::~ProcessContainer() { Clear(mMutex); };

int ProcessContainer::GetNumEntries() const { return (int)mEntries.size(); }

std::mutex &ProcessContainer::GetMutex() { return mMutex; }

std::vector<std::shared_ptr<ProcessEntry>> &ProcessContainer::GetEntries() {
  return mEntries;
}

std::shared_ptr<ProcessEntry> ProcessContainer::GetSelectedEntry() const {
  return mSelectedEntry;
}

std::shared_ptr<ProcessEntry>
ProcessContainer::FindEntry(const std::shared_ptr<ProcessEntry> &entry) const {
  if (!entry) {
    return nullptr;
  }
  return FindEntry(entry->GetId());
}

std::shared_ptr<ProcessEntry>
ProcessContainer::FindEntry(uint32_t procId) const {
  for (auto &proc : mEntries) {
    if (proc->GetId() == procId) {
      return proc;
    }
  }

  return nullptr;
}

void ProcessContainer::AddEntry(std::shared_ptr<ProcessEntry> &entry) {
  mEntries.emplace_back(entry);
  SetDirty();
}

void ProcessContainer::SelectEntry(const uint32_t procId, bool preserve) {
  if (const auto entry = FindEntry(procId); entry) {
    if (mSelectedEntry) {
      mSelectedEntry->Deselect();

      if (entry && (mSelectedEntry->GetId() == entry->GetId())) {
        if (preserve) {
          mSelectedEntry->Select();
        } else {
          mSelectedEntry = nullptr;
        }
        return;
      }
    }

    mSelectedEntry = entry;
    mSelectedEntry->Select();
  }
}

void ProcessContainer::SelectEntry(std::shared_ptr<ProcessEntry> &entry,
                                   bool preserve) {
  if (entry) {
    SelectEntry(entry->GetId(), preserve);
  } else {
    mSelectedEntry = nullptr;
  }
}

void ProcessContainer::EraseEntry(std::shared_ptr<ProcessEntry> &entry) {
  if (!entry) {
    return;
  }

  for (auto it = mEntries.begin(); it != mEntries.end(); ++it) {
    if (auto &proc = mEntries.at(
            reinterpret_cast<
                std::vector<std::shared_ptr<ProcessEntry>>::size_type>(&it));
        proc->GetId() == entry->GetId()) {
      proc.reset();
      proc = nullptr;
      break;
    }
  }

  SetDirty();
}

void ProcessContainer::Copy(ProcessContainer &other) {
  if (this == &other) {
    return;
  }

  Clear(mMutex);
  mSelectedEntry = nullptr;

  std::scoped_lock lock1(mMutex);
  std::scoped_lock lock2(other.GetMutex());

  for (const auto &entry : other.GetEntries()) {
    mEntries.emplace_back(std::make_shared<ProcessEntry>(entry));
  }

  SetDirty();
}

ProcessContainer &ProcessContainer::operator=(const ProcessContainer &other) {
  if (this != &other) {
    mEntries = other.mEntries;
    mDirty = other.mDirty;

    mSelectedEntry = other.mSelectedEntry;
  }

  return *this;
}

void ProcessContainer::UpdateEntries() {
  std::scoped_lock lock1(mMutex);
  for (auto &entry : mEntries) {
    entry->UpdatePerfStats();
  }
}

bool ProcessContainer::Contains(uint32_t procId) const {
  if (auto proc = FindEntry(procId)) {
    return true;
  }
  return false;
}
} // namespace RESANA
