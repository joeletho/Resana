#include "ProcessManager.h"

#include "helpers/WinFuncs.h"

namespace RESANA {

    ProcessManager *ProcessManager::sInstance = nullptr;
    std::vector<PROCESSENTRY32> ProcessManager::mProcesses{};

    ProcessManager::ProcessManager() = default;

    void ProcessManager::Init() {
        if (!sInstance) {
            sInstance = new ProcessManager();
            sInstance->Run();
        }
    }

    void ProcessManager::Run() {
        if (mRunning) { return; }

        mRunning = true;
        mProcThread = new std::thread(&ProcessManager::GetProcessList, this);
        mProcThread->detach();
    }

    void ProcessManager::Stop() {
        mRunning = false;
    }

    void ProcessManager::GetProcessList() {
        HANDLE hProcessSnap;
        HANDLE hProcess;
        PROCESSENTRY32 procEntry;
        DWORD priorityClass;
        std::vector<PROCESSENTRY32> procVec;

        while (mRunning) {
            // Take a snapshot of all processes in the system.
            hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hProcessSnap == INVALID_HANDLE_VALUE) {
                PrintWin32Error(TEXT("CreateToolhelp32Snapshot (of processes)"));
            }

            // Set the size of the structure before using it.
            procEntry.dwSize = sizeof(PROCESSENTRY32);

            // Retrieve information about the first process,
            // and exit if unsuccessful
            if (!Process32First(hProcessSnap, &procEntry)) {
                PrintWin32Error(TEXT("Process32First")); // show cause of failure
                CloseHandle(hProcessSnap);          // clean the snapshot object
            }

            // Now walk the snapshot of processes, and
            // display information about each process in turn
            do {
                RS_CORE_INFO("=====================================================");
                RS_CORE_INFO("PROCESS NAME:  {0}", procEntry.szExeFile);
                RS_CORE_INFO("-------------------------------------------------------");

                // Retrieve the priority class.
                priorityClass = 0;
                hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry.th32ProcessID);
                if (!hProcess) {
                    PrintWin32Error(TEXT("OpenProcess"));
                } else {
                    priorityClass = GetPriorityClass(hProcess);
                    if (!priorityClass) {
                        PrintWin32Error(TEXT("GetPriorityClass"));
                    }
                    CloseHandle(hProcess);
                }

                RS_CORE_INFO("  Process ID        = 0x{0}", procEntry.th32ProcessID);
                RS_CORE_INFO("  Thread count      = {0}", procEntry.cntThreads);
                RS_CORE_INFO("  Parent process ID = 0x{0}", procEntry.th32ParentProcessID);
                RS_CORE_INFO("  Priority base     = {0}", procEntry.pcPriClassBase);
                if (priorityClass) {
                    RS_CORE_INFO("  Priority class    = {0}", priorityClass);
                }

                // List the modules and threads associated with this process
                // These are expensive, so we create new threads for each function
                std::thread th1(ProcessManager::ListProcessModules, procEntry.th32ProcessID);
                std::thread th2(ProcessManager::ListProcessThreads, procEntry.th32ProcessID);
                th1.join();
                th2.join();

                procVec.push_back(procEntry);

            } while (Process32Next(hProcessSnap, &procEntry));
            
            mProcesses = procVec;
            procVec.clear();
            CloseHandle(hProcessSnap);
        }
    }

    void ProcessManager::ListProcessModules(DWORD pid) {
        HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
        MODULEENTRY32 moduleEntry;

        // Take a snapshot of all modules in the specified process.
        hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
        if (hModuleSnap == INVALID_HANDLE_VALUE) {
            PrintWin32Error(TEXT("CreateToolhelp32Snapshot (of modules)"));
        }

        // Set the size of the structure before using it.
        moduleEntry.dwSize = sizeof(MODULEENTRY32);

        // Retrieve information about the first module,
        // and exit if unsuccessful
        if (!Module32First(hModuleSnap, &moduleEntry)) {
            PrintWin32Error(TEXT("Module32First"));  // show cause of failure
            CloseHandle(hModuleSnap);           // clean the snapshot object
        }

        // Now walk the module list of the process,
        // and display information about each module
        do {
            RS_CORE_INFO("     MODULE NAME:     {0}", moduleEntry.szModule);
            RS_CORE_INFO("     Executable     = {0}", moduleEntry.szExePath);
            RS_CORE_INFO("     Process ID     = 0x{0}", moduleEntry.th32ProcessID);
            RS_CORE_INFO("     Ref count (g)  = 0x{0}", moduleEntry.GlblcntUsage);
            RS_CORE_INFO("     Ref count (p)  = 0x{0}", moduleEntry.ProccntUsage);
            RS_CORE_INFO("     Base address   = 0x{0}", (DWORD) moduleEntry.modBaseAddr);
            RS_CORE_INFO("     Base size      = {0}", moduleEntry.modBaseSize);

        } while (Module32Next(hModuleSnap, &moduleEntry));

        CloseHandle(hModuleSnap);
    }

    void ProcessManager::ListProcessThreads(DWORD ownerPID) {
        HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
        THREADENTRY32 threadEntry;

        // Take a snapshot of all running threads
        hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hThreadSnap == INVALID_HANDLE_VALUE) {
            RS_CORE_WARN("CreateToolhelp32Snapshot invalid handle value!");
        }

        // Fill in the size of the structure before using it.
        threadEntry.dwSize = sizeof(THREADENTRY32);

        // Retrieve information about the first thread,
        // and exit if unsuccessful
        if (!Thread32First(hThreadSnap, &threadEntry)) {
            PrintWin32Error(TEXT("Thread32First")); // show cause of failure
            CloseHandle(hThreadSnap);          // clean the snapshot object
        }

        // Now walk the thread list of the system,
        // and display information about each thread
        // associated with the specified process
        do {
            if (threadEntry.th32OwnerProcessID == ownerPID) {
                RS_CORE_INFO("     THREAD ID      = 0x{0}", threadEntry.th32ThreadID);
                RS_CORE_INFO("     Base priority  = {0}", threadEntry.tpBasePri);
                RS_CORE_INFO("     Delta priority = {0}", threadEntry.tpDeltaPri);
            }
        } while (Thread32Next(hThreadSnap, &threadEntry));

        CloseHandle(hThreadSnap);
    }

} // RESANA