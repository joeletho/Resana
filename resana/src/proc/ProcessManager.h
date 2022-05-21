#pragma once

#include <Windows.h>

#include <thread>
#include <vector>

#include <TlHelp32.h>

namespace RESANA {

    class ProcessManager {
    public:
        static void Init();

        void Run();
        void Stop();


        void GetProcessList();
        static void ListProcessModules(DWORD pid);
        static void ListProcessThreads(DWORD ownerPID);

        static std::vector<PROCESSENTRY32>& GetProcesses() { return mProcesses; }

        [[nodiscard]] bool IsRunning() const { return mRunning; }

    private:
        ProcessManager();
        ~ProcessManager() = default;

    private:
        static std::vector<PROCESSENTRY32> mProcesses;
        bool mRunning = false;

        std::thread *mProcThread = nullptr;
        std::thread *mProcModulesThread = nullptr;
        std::thread *mProcThreadsThread = nullptr;

        static ProcessManager *sInstance;
    };

} // RESANA
