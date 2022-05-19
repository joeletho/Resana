#pragma once

#include <TCHAR.h>
#include <Pdh.h>

#include "PerfManager.h"

#include <set>
#include <deque>

#include <helpers/Time.h>

namespace RESANA {

    struct ProcessorData {
        PDH_FMT_COUNTERVALUE_ITEM *ProcessorPtr = nullptr;    // Array of PDH_FMT_COUNTERVALUE_ITEM structures
        std::set<std::pair<int, double>> *Set{};
        DWORD Size{};
        DWORD Buffer{};                                     // Size of the sProcessors buffer
    };

    class CPUData {
    public:
        static void Init();

        void Run();
        void Stop();

        [[nodiscard]] double GetCurrentLoad();
        [[nodiscard]] double GetCurrentLoadProc() const;
        [[nodiscard]] double GetCurrentLoadTotal() const;
        [[nodiscard]] double GetCurrentLoadTotalProc() const;
        [[nodiscard]] std::set<std::pair<int, double>> GetCurrentLoadAll() const;
        [[nodiscard]] int GetNumProcessors() const;

        [[nodiscard]] bool IsRunning() const { return mRunning; }

    private:
        CPUData();
        ~CPUData();
        void InitData();
        void InitProc();

        void CollectData();
        void UpdateTotalLoad();
        void UpdateTotalLoadProc();

        double CalculateAvgLoad();
    private:
        unsigned long mTimeStarted{};
        ProcessorData mProcessorData{};

        std::set<std::pair<int, double>> mCPUSet;
        std::deque<double> mLoadDeque;
        double mCurrentLoadCPU;
        double mLastLoadCPU;
        double mCurrentLoadProc;
        double mLastLoadProc;

        bool mRunning = false;

        std::thread *mCollectionThread = nullptr;
        std::thread *mUpdateTotalThread = nullptr;
        std::thread *mUpdateTotalProcThread = nullptr;

        /* DATA */
        HQUERY mDataQuery;
        PDH_HCOUNTER mDataCounter;
        PDH_FMT_COUNTERVALUE mCounter;

        /* PROC */
        HANDLE mHandle;
        HQUERY mProcQuery;
        PDH_HCOUNTER mProcCounter;
        ULARGE_INTEGER mLastCPU, mLastSysCPU, mLastUserCPU;

        static CPUData *sInstance;

        friend class PerfManager;
    };

} // RESANA

