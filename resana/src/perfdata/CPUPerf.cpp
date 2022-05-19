#include "CPUData.h"

#include <Windows.h>

#include <PdhMsg.h>
#include <core/Core.h>

#include "helpers/WinFuncs.h"

namespace RESANA {

    CPUData *CPUData::sInstance = nullptr;

    CPUData::CPUData() {
        mTimeStarted = Time::GetTime();
        InitData();
        InitProc();
    };

    CPUData::~CPUData() {
        if (mProcessorData.ProcessorPtr) {
            free(mProcessorData.ProcessorPtr);
        }
    }

    void CPUData::InitData() {
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

    void CPUData::CollectData() {
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

            Time::Sleep(1000);

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
                    if (mLoadDeque.size() == 3) {
                        mLoadDeque.pop_front();
                    }
                    mLoadDeque.push_back(value);
                    mCurrentLoadCPU = CalculateAvgLoad();
                } else {
                    // Use set to auto sort in ascending order by name. std::map would not sort correctly.
                    mProcessorData.Set->emplace(std::stoi(name), value);
                }
                printf("counter: %s, value %.2g\n", name.c_str(), value);
            }

            mCPUSet = *mProcessorData.Set;
            mProcessorData.ProcessorPtr = nullptr;
            mProcessorData.Buffer = 0;
        }
    }

    double CPUData::CalculateAvgLoad() {
        double sum = 0;
        for (auto val: mLoadDeque) {
            sum += val;
        }
        return sum / mLoadDeque.size();
    }

    std::set<std::pair<int, double>> CPUData::GetCurrentLoadAll() const {
        return mCPUSet;
    }

    void CPUData::Init() {
        if (!sInstance) {
            sInstance = new CPUData();
            sInstance->Run();
        }
    }

    double CPUData::GetCurrentLoad() {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadCPU;
    }

    double CPUData::GetCurrentLoadProc() const {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadProc;
    }

    double CPUData::GetCurrentLoadTotal() const {
        if (mCurrentLoadCPU < 0) {
            return 0.0;
        }
        return mCurrentLoadCPU * GetNumProcessors();
    }

    double CPUData::GetCurrentLoadTotalProc() const {
        if (mCurrentLoadProc < 0) {
            return 0.0;
        }
        return mCurrentLoadProc * GetNumProcessors();
    }

    int CPUData::GetNumProcessors() const {
        return (int) mProcessorData.Size;
    }

    void CPUData::Run() {
        if (mRunning) { return; }

        mRunning = true;
        mCollectionThread = new std::thread(&CPUData::CollectData, this);
        mCollectionThread->detach();
        // mUpdateTotalThread = new std::thread(&CPUData::UpdateTotalLoad, this);
        // mUpdateTotalThread->detach();
        mUpdateTotalProcThread = new std::thread(&CPUData::UpdateTotalLoadProc, this);
        mUpdateTotalProcThread->detach();
    }

    void CPUData::Stop() {
        mRunning = false;
    }

    void CPUData::UpdateTotalLoad() {
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
            Time::Sleep(1000);

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

    void CPUData::InitProc() {
        FILETIME ftime, fsys, fuser;

        GetSystemTimeAsFileTime(&ftime);
        memcpy(&mLastCPU, &ftime, sizeof(FILETIME));

        mHandle = GetCurrentProcess();
        GetProcessTimes(mHandle, &ftime, &ftime, &fsys, &fuser);
        memcpy(&mLastSysCPU, &fsys, sizeof(FILETIME));
        memcpy(&mLastUserCPU, &fuser, sizeof(FILETIME));
    }

    void CPUData::UpdateTotalLoadProc() {
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