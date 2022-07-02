#pragma once

#include "ProcessContainer.h"

#include <mutex>
#include <string>

#include <TlHelp32.h>

namespace RESANA {

class ProcessEntry {
    typedef unsigned long ulong;

    struct Process {
        std::string Name {};
        ulong ProcessId {};
        ulong ParentProcessId {};
        ulong ModuleId {};
        ulong MemoryUsage {};
        ulong ThreadCount {};
        ulong PriorityClass {};
        ulong Flags {};

        explicit Process(const PROCESSENTRY32& pe32)
        {
            Name.assign(pe32.szExeFile);
            ProcessId = pe32.th32ProcessID;
            ParentProcessId = pe32.th32ParentProcessID;
            ModuleId = pe32.th32ModuleID;
            MemoryUsage = pe32.cntUsage;
            ThreadCount = pe32.cntThreads;
            PriorityClass = pe32.pcPriClassBase;
            Flags = pe32.dwFlags;
        }

        explicit Process(const ProcessEntry* other)
        {
            Name.assign(other->GetName());
            ProcessId = other->GetProcessId();
            ParentProcessId = other->GetParentProcessId();
            ModuleId = other->GetModuleId();
            MemoryUsage = other->GetMemoryUsage();
            ThreadCount = other->GetThreadCount();
            PriorityClass = other->GetPriorityClass();
            Flags = other->GetFlags();
        }
    };

public:
    explicit ProcessEntry(const PROCESSENTRY32& pe32)
        : mProcess(pe32)
        , mLock(mMutex, std::defer_lock)
    {
        this->operator=(pe32);
    }

    explicit ProcessEntry(const ProcessEntry* entry)
        : mProcess(entry)
        , mLock(mMutex, std::defer_lock)
    {
        this->operator=(entry);
    }

    ~ProcessEntry()
    {
        // Acquire mutex and release upon destruction.
        std::scoped_lock lock(mMutex);
    }

    [[nodiscard]] ulong GetMemoryUsage() const { return mProcess.MemoryUsage; }
    [[nodiscard]] ulong GetProcessId() const { return mProcess.ProcessId; }
    [[nodiscard]] ulong GetModuleId() const { return mProcess.ModuleId; }
    [[nodiscard]] ulong GetThreadCount() const { return mProcess.ThreadCount; }
    [[nodiscard]] ulong GetParentProcessId() const { return mProcess.ParentProcessId; }
    [[nodiscard]] ulong GetFlags() const { return mProcess.Flags; }
    [[nodiscard]] std::string GetName() const { return mProcess.Name; }
    [[nodiscard]] ulong GetPriorityClass() const { return mProcess.PriorityClass; }

    void Free() { this->~ProcessEntry(); }

    [[nodiscard]] bool IsSelected() const { return mSelected; }
    bool& Running() { return mRunning; }
    std::mutex& Mutex() { return mMutex; }

    // Overloads
    ProcessEntry& operator=(const ProcessEntry* entry)
    {
        mProcess.Name.assign(entry->GetName());
        mProcess.ProcessId = entry->GetProcessId();
        mProcess.ParentProcessId = entry->GetParentProcessId();
        mProcess.ModuleId = entry->GetModuleId();
        mProcess.MemoryUsage = entry->GetMemoryUsage();
        mProcess.ThreadCount = entry->GetThreadCount();
        mProcess.PriorityClass = entry->GetPriorityClass();
        mProcess.Flags = entry->GetFlags();
        mSelected = entry->IsSelected();
        return *this;
    }

    ProcessEntry& operator=(const PROCESSENTRY32& pe32)
    {
        mProcess.Name.assign(pe32.szExeFile);
        mProcess.ProcessId = pe32.th32ProcessID;
        mProcess.ParentProcessId = pe32.th32ParentProcessID;
        mProcess.ModuleId = pe32.th32ModuleID;
        mProcess.MemoryUsage = pe32.cntUsage;
        mProcess.ThreadCount = pe32.cntThreads;
        mProcess.PriorityClass = pe32.pcPriClassBase;
        mProcess.Flags = pe32.dwFlags;
        return *this;
    }

    bool operator==(const PROCESSENTRY32 pe32) const
    {
    	return mProcess.Name == pe32.szExeFile ||
			mProcess.ProcessId == pe32.th32ProcessID ||
			mProcess.ParentProcessId == pe32.th32ParentProcessID ||
			mProcess.ModuleId == pe32.th32ModuleID ||
			mProcess.MemoryUsage == pe32.cntUsage ||
			mProcess.ThreadCount == pe32.cntThreads ||
			mProcess.PriorityClass == pe32.pcPriClassBase ||
			mProcess.Flags == pe32.dwFlags;
    }

    bool operator!=(const PROCESSENTRY32& pe32) const
    {
    	return mProcess.Name != pe32.szExeFile ||
			mProcess.ProcessId != pe32.th32ProcessID ||
			mProcess.ParentProcessId != pe32.th32ParentProcessID ||
			mProcess.ModuleId != pe32.th32ModuleID ||
			mProcess.MemoryUsage != pe32.cntUsage ||
			mProcess.ThreadCount != pe32.cntThreads ||
			mProcess.PriorityClass != pe32.pcPriClassBase ||
			mProcess.Flags != pe32.dwFlags;
    }

private:
    void Select() { mSelected = true; }
    void Deselect() { mSelected = false; }

private:
    Process mProcess;
    std::mutex mMutex {};
    std::unique_lock<std::mutex> mLock {};
    bool mRunning = true;
    bool mSelected = false;

    friend class ProcessManager;
    friend class ProcessContainer;
};

}
