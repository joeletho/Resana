#pragma once

#include <TCHAR.h>
#include <Pdh.h>

#include "PerfManager.h"

#include <set>
#include <deque>

#include <helpers/Time.h>

namespace RESANA {

    class CPUPerf {
        struct ProcessorData {
            PDH_FMT_COUNTERVALUE_ITEM *ProcessorPtr = nullptr;  // Array of PDH_FMT_COUNTERVALUE_ITEM structures
            std::set<std::pair<int, double>> *Set{};            // Set containing current logical cpu values
            DWORD Size{};                                       // Number of processors including _Total
            DWORD Buffer{};                                     // Size of the sProcessors buffer
        };
    public:

        /**
         * Init constructs a static instance of CPUPerf and enables update loops.
         * @returns void. */
        static void Init();

        /**
         * Run creates and detaches the update threads.
         * @returns void. */
        void Run();

        /**
         * Stop flags the update loops to stop.
         * @returns void. */
        void Stop();

        /**
         * GetNumProcessors returns the number of CPUs on the system.
         * @returns The number of processors on the system. */
        [[nodiscard]] int GetNumProcessors() const;

        /**
         * GetCurrentLoad returns the current CPU load on the system.
         * @returns The current CPU load on the system. */
        [[nodiscard]] double GetCurrentLoad() const;

        /**
         * GetCurrentLoadProc returns the current CPU load of the process.
         * @returns the current CPU load of the process. */
        [[nodiscard]] double GetCurrentLoadProc() const;

        /**
         * GetCurrentLoadAll returns a set of all current CPU names and loads.
         * @returns a set of all current CPU loads. */
        [[nodiscard]] std::set<std::pair<int, double>> GetCurrentLoadAll() const;

        /**
         * IsRunning returns the current state of the update loops.
         * @returns true is the update loops are currently running, otherwise false. */
        [[nodiscard]] bool IsRunning() const { return mRunning; }

    private:
        /**
         * CPUPerf calls InitData and InitProc. */
        CPUPerf();

        /**
         * ~CPUPerf frees memory, deletes all threads and static instance. */
        ~CPUPerf();

        /**
         * InitData sets mProcessor.Size and instantiates mProcessor.Set, then opens the PDH query and adds the counter.
         * @returns void. */
        void InitData();

        /**
         * Init gets the handle for the current process and gets the process times.
         * @returns void. */
        void InitProc();

        /**
         * CollectData is the main update loop. It is responsible for collecting the query data and extracting CPU usage and current load.
         * @returns void. */
        void CollectData();

        /**
         * UpdateTotalLoad updates the current load of the CPU _Total only.
         * @returns void. */
        void UpdateTotalLoad();

        /**
         * UpdateTotalLoadProc is a update loop that updates the total CPU load of the current process.
         * @returns void. */
        void UpdateTotalLoadProc();

        /**  CalculateAvgLoad calculates and returns the average CPU load.
         * @returns The average CPU load. */
        double CalculateAvgLoad();

    private:
        const int MIN_SLEEP_TIME = 1000;
        const int DEQUE_SIZE = 3;

        ProcessorData mProcessorData{};
        std::set<std::pair<int, double>> mCPUSet;
        std::deque<double> mLoadDeque;

        double mCurrentLoadCPU{};
        double mCurrentLoadProc{};
        bool mRunning = false;

        /* DATA */
        HQUERY mDataQuery{};
        PDH_HCOUNTER mDataCounter{};
        PDH_FMT_COUNTERVALUE mCounter{};

        /* PROC */
        HANDLE mHandle{};
        HQUERY mProcQuery{};
        PDH_HCOUNTER mProcCounter{};
        ULARGE_INTEGER mLastCPU{}, mLastSysCPU{}, mLastUserCPU{};

        std::thread *mCollectionThread = nullptr;
        std::thread *mUpdateTotalThread = nullptr;
        std::thread *mUpdateTotalProcThread = nullptr;

        static CPUPerf *sInstance;

        friend class PerfManager;
    };

} // RESANA

