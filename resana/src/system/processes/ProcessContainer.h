#pragma once

#include <mutex>
#include <vector>

#include "ProcessEntry.h"

namespace RESANA {
class ProcessEntry;

class ProcessContainer {
public:
    ProcessContainer();
    explicit ProcessContainer(const ProcessContainer* other);
    ~ProcessContainer();

    std::mutex& GetMutex();
    [[nodiscard]] int GetNumEntries() const;

    std::vector<ProcessEntry*>& GetEntries();
    [[nodiscard]] ProcessEntry* GetSelectedEntry() const;
    [[nodiscard]] ProcessEntry* FindEntry(const ProcessEntry* entry) const;
    [[nodiscard]] ProcessEntry* FindEntry(uint32_t procId) const;

    void AddEntry(ProcessEntry* entry);
    void SelectEntry(uint32_t procId, bool preserve = false);
    void SelectEntry(ProcessEntry* entry, bool preserve = false);
    void EraseEntry(const ProcessEntry* entry);
    void Copy(ProcessContainer* other);

    void SetClean() { mDirty = false; }
    void SetDirty() { mDirty = true; }
    bool IsDirty() const { return mDirty; }

    template <class _MutTy>
    void Clear(_MutTy& mutex);

private:
    std::mutex mMutex {};
    std::vector<ProcessEntry*> mEntries {};
    bool mDirty = false;

    ProcessEntry* mSelectedEntry = nullptr;
};

template <class _MutTy>
void ProcessContainer::Clear(_MutTy& mutex)
{
    std::scoped_lock slock(mutex);
    for (auto entry : mEntries) {
        delete entry;
        entry = nullptr;
    }

    mEntries.clear();
}

}
