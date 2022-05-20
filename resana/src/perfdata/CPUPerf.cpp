#include "CPUPerf.h"

#include <Windows.h>

#include <PdhMsg.h>
#include <core/Core.h>

#include "helpers/WinFuncs.h"

namespace RESANA {

    CPUPerf *CPUPerf::sInstance = nullptr;

    CPUPerf::CPUPerf() {
        mTimeStarted = Time::GetTime();
        InitData();
        InitProc();
    };

    CPUPerf::~CPUPerf() {
        if (mProcessorData.ProcessorPtr) {
            free(mProcessorData.ProcessorPtr);
        }
        mCollectionThread = nullptr;
        mUpdateTotalThread = nullptr;
        mUpdateTotalProcThread = nullptr;
        sInstance = nullptr;
        delete mCollectionThread;
        delete mUpdateTotalThread;
        delete mUpdateTotalProcThread;
        delete sInstance;
    }

    void CPUPerf::InitData() {
        PDH_STATUS pdhStatus = ERROR_SUCCESS;
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        mProcessorData.Size = (int) sysInfo.dwNumberOfProcessors;
        mProcessorData.Set = new std::set<std::pair<int, double>>();

        pdhStatus = PdhOpenQuery(nullptr, 0, &mDataQuery);
        if (pdhStatus != ERROR_SUCCESS) {
            RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
        }

        // Specify a counter object with a wildcard for the instance.
        pdhStatus = PdhAddCounter(mDataQuery, TEXT("\\Processor(*)\\% Processor Time"), 0, &mDataCounter);
        if (pdhStatus != ERROR_SUCCESS) {
            RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
        }
    }

    void CPUPerf::CollectData() {
        PDH_STATUS pdhStatus = ERROR_SUCCESS;
        while (mRunning) {
            if (!mProcessorData.Set->empty()) {
                mProcessorData.Set->clear();
            }

            // Some counters need two sample in order to format a value, so
            // make this call to get the first value before entering the loop.
            pdhStatus = PdhCollectQueryData(mDataQuery);
            if (pdhStatus != ERROR_SUCCESS) {
                RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
            }

            Time::Sleep(MIN_SLEEP_TIME);

            pdhStatus = PdhCollectQueryData(mDataQuery);
            if (pdhStatus != ERROR_SUCCESS) {
                RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
            }

            // Get the required size of the pDataProcessors buffer.
            pdhStatus = PdhGetFormattedCounterArray(mDataCounter, PDH_FMT_DOUBLE, &mProcessorData.Buffer,
                                                    &mProcessorData.Size, mProcessorData.ProcessorPtr);
            if (pdhStatus != PDH_MORE_DATA) {
                RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
            }

            mProcessorData.ProcessorPtr = (PDH_FMT_COUNTERVALUE_ITEM *) malloc(mProcessorData.Buffer);
            if (!mProcessorData.ProcessorPtr) {
                RS_CORE_ERROR("malloc for PdhGetFormattedCounterArray failed 0x{0}", pdhStatus);
            }
            pdhStatus = PdhGetFormattedCounterArray(mDataCounter, PDH_FMT_DOUBLE, &mProcessorData.Buffer,
                                                    &mProcessorData.Size,
                                                    mProcessorData.ProcessorPtr);
            if (pdhStatus != ERROR_SUCCESS) {
                RS_CORE_ERROR("PdhGetFormattedCounterArray failed with 0x{0}", pdhStatus);
            }
            // Loop through the array and print the instance name and counter value.
            for (DWORD i = 0; i < mProcessorData.Size; i++) {
                std::string name = mProcessorData.ProcessorPtr[i].szName;
                double value = mProcessorData.ProcessorPtr[i].FmtValue.doubleValue;
                if (name == "_Total") {
                    // Put the total into a deque to compute the average
                    if (mLoadDeque.size() == DEQUE_SIZE) {
                        mLoadDeque.pop_front();
                    }
                    mLoadDeque.push_back(value);
                    mCurrentLoadCPU = CalculateAvgLoad();
                } else {
                    // Use set to auto sort in ascending order by name. std::map would not sort correctly.
                    mProcessorData.Set->emplace(std::stoi(name), value);
                }
                RS_CORE_TRACE("counter: cpu {0}, value {1:.2}", name.c_str(), value);
            }

            mCPUSet = *mProcessorData.Set;
            mProcessorData.ProcessorPtr = nullptr;
            mProcessorData.Buffer = 0;
        }
    }

    double CPUPerf::CalculateAvgLoad() {
        double sum = 0;
        for (auto val: mLoadDeque) {
            sum += val;
        }
        return sum / mLoadDeque.size();
    }

    std::set<std::pair<int, double>> CPUPerf::GetCurrentLoadAll() const {
        return mCPUSet;
    }

    void CPUPerf::Init() {
        if (!sInstance) {
            sInstance = new CPUPerf();
            sInstance->Run();
        }
    }

    double CPUPerf::GetCurrentLoad() {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadCPU;
    }

    double CPUPerf::GetCurrentLoadProc() const {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadProc;
    }

    double CPUPerf::GetCurrentLoadTotal() const {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadCPU * GetNumProcessors();
    }

    double CPUPerf::GetCurrentLoadTotalProc() const {
        if (mCurrentLoadProc < 0) {
            return 0.0;
        }
        return mCurrentLoadProc * GetNumProcessors();
    }

    int CPUPerf::GetNumProcessors() const {
        return (int) mProcessorData.Size;
    }

    void CPUPerf::Run() {
        if (mRunning) { return; }

        mRunning = true;
        mCollectionThread = new std::thread(&CPUPerf::CollectData, this);
        mCollectionThread->detach();
        // mUpdateTotalThread = new std::thread(&CPUData::UpdateTotalLoad, this);
        // mUpdateTotalThread->detach();
        mUpdateTotalProcThread = new std::thread(&CPUPerf::UpdateTotalLoadProc, this);
        mUpdateTotalProcThread->detach();
    }

    void CPUPerf::Stop() {
        mRunning = false;
    }

    void CPUPerf::UpdateTotalLoad() {
        PDH_STATUS pdhStatus = ERROR_SUCCESS;

        pdhStatus = PdhOpenQuery(nullptr, 0, &mProcQuery);
        if (pdhStatus != ERROR_SUCCESS) {
            RS_CORE_ERROR("PdhOpenQuery failed with 0x{0}", pdhStatus);
        }

        // Specify a counter object with a wildcard for the instance.
        pdhStatus = PdhAddCounter(mProcQuery, TEXT("\\Processor(_Total)\\% Processor Time"), 0, &mProcCounter);
        if (pdhStatus != ERROR_SUCCESS) {
            RS_CORE_ERROR("PdhAddCounter failed with 0x{0}", pdhStatus);
        }

        while (mRunning) {
            pdhStatus = PdhCollectQueryData(mProcQuery);
            if (pdhStatus != ERROR_SUCCESS) {
                RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
            }
            Time::Sleep(MIN_SLEEP_TIME);

            pdhStatus = PdhCollectQueryData(mProcQuery);
            if (pdhStatus == ERROR_SUCCESS) {
                pdhStatus = PdhGetFormattedCounterValue(mProcCounter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, nullptr,
                                                        &mCounter);
                if (pdhStatus == PDH_CALC_NEGATIVE_DENOMINATOR) {
                    RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
                }
            } else {
                RS_CORE_ERROR("PdhCollectQueryData failed with 0x{0}", pdhStatus);
            }

            mCurrentLoadCPU = mCounter.doubleValue;
        }
    }

    void CPUPerf::InitProc() {
        FILETIME ftime, fsys, fuser;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&mLastCPU, &ftime, sizeof(FILETIME));

        mHandle = GetCurrentProcess();
        GetProcessTimes(mHandle, &ftime, &ftime, &fsys, &fuser);
        memcpy(&mLastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&mLastUserCPU, &fuser, sizeof(FILETIME));
    }

    void CPUPerf::UpdateTotalLoadProc() {
        FILETIME ftime, fsys, fuser;
        ULARGE_INTEGER now, sys, user;
        double percent;

        while (mRunning) {
            Time::Sleep(1000);
            GetSystemTimeAsFileTime(&ftime);
            memcpy(&now, &ftime, sizeof(FILETIME));

            GetProcessTimes(mHandle, &ftime, &ftime, &fsys, &fuser);
            memcpy(&sys, &fsys, sizeof(FILETIME));
            memcpy(&user, &fuser, sizeof(FILETIME));
            percent = (double) (sys.QuadPart - mLastSysCPU.QuadPart) +
                      (double) (user.QuadPart - mLastUserCPU.QuadPart);
            percent /= (double) (now.QuadPart - mLastCPU.QuadPart);
            percent /= GetNumProcessors();
            mLastCPU = now;
            mLastUserCPU = user;
            mLastSysCPU = sys;
            mCurrentLoadProc = percent * 100;
        }
    }

} // RESANA