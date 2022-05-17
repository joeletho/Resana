#pragma once

#include "rspch.h"

namespace RESANA {

#define CLOCKS_PER_HOUR 3600000;
#define CLOCKS_PER_MIN 60000;

    //--------------------------------------------------------------
    // [SECTION] Time
    //--------------------------------------------------------------

    class Time {
    public:
        static float GetTimeSeconds();
        static float GetTimeMilliseconds();
        static float GetTimeNanoseconds();

        static std::string GetTimeFormatted();
        static void Sleep(int sleep_time_ms);

    private:
        Time();
    private:
        static std::chrono::steady_clock::time_point mTimeStarted;

        [[maybe_unused]] static Time *sInstance;
    };

    class Timestep {
    public:
        explicit Timestep(float time = 0.0f)
                : mTime(time) {
        }

        operator float() const { return mTime; }

        [[nodiscard]] float GetSeconds() const { return mTime; }

        [[nodiscard]] float GetMilliseconds() const { return mTime * 1000.0f; }

    private:
        float mTime;
    };


    //--------------------------------------------------------------
    // [SECTION] Stop Watch
    //--------------------------------------------------------------

    class StopWatch {
    public:
        void Start();
        void Stop();
        void CalculateTime();
        [[nodiscard]] float GetSeconds() const;
        [[nodiscard]] unsigned int GetHours() const;
        [[nodiscard]] unsigned int GetMinutes() const;
        [[nodiscard]] unsigned int GetMilliseconds() const;
        std::string GetElapsedTime();

        void CalculateElapsedTime();

    private:
        StopWatch();

    private:
        std::string sElapsedTime;
        std::chrono::steady_clock::time_point sStartTime;
        std::chrono::steady_clock::time_point sStopTime;

    private:
        struct TimeData {
            unsigned int Hours;
            unsigned int Minutes;
            unsigned long Milliseconds;
            unsigned long Nanoseconds;
            float Seconds;
        };
        TimeData sData{};
    };

}

