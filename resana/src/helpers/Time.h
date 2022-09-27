#pragma once

#include <string>

namespace RESANA {

#define CLOCKS_PER_HOUR 3600000;
#define CLOCKS_PER_MIN 60000;

//--------------------------------------------------------------
// [SECTION] Time
//--------------------------------------------------------------

class Time {
public:
  static void Start();
  static void Stop();
  static float GetTimeSeconds();
  static float GetTimeMilliseconds();
  static float GetTimeNanoseconds();

  static long long GetTime();
  static std::string GetTimeFormatted();

  template <typename N> static bool Sleep(N sleepTime_ms) {
    std::this_thread::sleep_for(std::chrono ::milliseconds((int)sleepTime_ms));
    return true;
  }

private:
  Time();
  ~Time();

private:
  static std::chrono::steady_clock::time_point mTimeStarted;

  [[maybe_unused]] static Time *sInstance;
};

class Timestep {
public:
  Timestep(float time = 0.0f) : mTime(time) {}

  operator float() const { return mTime; }

  Timestep &operator=(const int ms) {
    mTime = ms / 1000.0f;
    return *this;
  }

  [[nodiscard]] float GetSeconds() const { return mTime; }

  [[nodiscard]] float GetMilliseconds() const { return mTime * 1000.0f; }

  bool operator>(const Timestep rhs) const { return mTime > rhs; };

private:
  float mTime;
};

class TimeTick {
public:
  enum Rate { Fast = 500, Normal = 1000, Slow = 2000 };
  inline static Rate Rate;

public:
  TimeTick();
  ~TimeTick();
  void Start();
  void Stop();

  Timestep GetTickCount() const { return mTick; }

private:
  void CountTicks();

  Timestep mTick = 0;
  bool mRunning = false;
};

//--------------------------------------------------------------
// [SECTION] Stop Watch
//--------------------------------------------------------------

class StopWatch {
public:
  StopWatch();

  void Start();
  void Stop();
  [[nodiscard]] float GetSeconds() const;
  [[nodiscard]] unsigned int GetHours() const;
  [[nodiscard]] unsigned int GetMinutes() const;
  [[nodiscard]] unsigned int GetMilliseconds() const;
  std::string GetElapsedTime();

private:
  void CalculateTime();
  void CalculateElapsedTime();

private:
  std::string sElapsedTime;
  std::chrono::steady_clock::time_point sStartTime;
  std::chrono::steady_clock::time_point sStopTime;
  bool mStarted = false;

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

} // namespace RESANA
