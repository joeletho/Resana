#include "Time.h"

#include "core/Application.h"

namespace RESANA {

//--------------------------------------------------------------
// [SECTION] Time
//--------------------------------------------------------------
Time *Time::sInstance = nullptr;
std::chrono::steady_clock::time_point Time::mTimeStarted;
std::time_t sSeed;

Time::Time() {
  sSeed = time(nullptr);
  mTimeStarted = std::chrono::steady_clock::now();
}

Time::~Time() {
  RS_CORE_TRACE("Time destroyed");
}

float Time::GetTimeSeconds() {
  return (float)((double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::chrono::high_resolution_clock::now() - mTimeStarted)
                     .count() *
                 1E-9);
};

float Time::GetTimeMilliseconds() {
  return (float)((double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::chrono::high_resolution_clock::now() - mTimeStarted)
                     .count() *
                 1E-3);
};

float Time::GetTimeNanoseconds() {
  return (float)((double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                     std::chrono::high_resolution_clock::now() - mTimeStarted)
                     .count() *
                 1E-6);
};

long long Time::GetTime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::high_resolution_clock::now() - mTimeStarted)
      .count();
}

std::string Time::GetTimeFormatted() {
  static char time_buf[50];
#pragma warning(                                                               \
    disable : 4996) // Disable std::asctime and std::localtime depreciation
  snprintf(time_buf, sizeof(time_buf), "%s",
           std::asctime(std::localtime(&sSeed)));
#pragma warning(default : 4996)
  return time_buf;
}
void Time::Start() {
  if (!sInstance) {
    sInstance = new Time();
  }
}
void Time::Stop() { delete sInstance; }

//--------------------------------------------------------------
// [SECTION] TimeTick
//--------------------------------------------------------------

TimeTick::TimeTick() { Start(); }

TimeTick::~TimeTick() {
  Stop();
  Time::Sleep(1000);
  RS_CORE_TRACE("TimeTick destroyed");
}

void TimeTick::Stop() { mRunning = false; }

void TimeTick::Start() {
  if (!mRunning) {
    mRunning = true;
    const auto &app = Application::Get();
    auto &threadPool = app.GetThreadPool();
    threadPool.Queue([&] { CountTicks(); });
  }
}

void TimeTick::CountTicks() {
  unsigned long count = 0;

  while (mRunning) {
    const auto secondsPassed = std::floor(Time::GetTimeSeconds());
    count = (secondsPassed > count) ? secondsPassed : count;
    if (count > mTick) {
      mTick = static_cast<Timestep>(count);
      Time::Sleep(1000);
    } else {
      Time::Sleep(10);
    }
  }
}

//--------------------------------------------------------------
// [SECTION] Stop Watch
//--------------------------------------------------------------

StopWatch::StopWatch() = default;

void StopWatch::Start() {
  sStartTime = std::chrono::steady_clock::now();
  mStarted = true;
}

void StopWatch::Stop() {
  sStopTime = std::chrono::steady_clock::now();
  mStarted = false;
  CalculateTime();
  CalculateElapsedTime();
}

void StopWatch::CalculateTime() {
  auto total = std::chrono::duration_cast<std::chrono::milliseconds>(sStopTime -
                                                                     sStartTime)
                   .count();
  unsigned long delta = total;
  sData.Hours = delta / CLOCKS_PER_HOUR;
  delta = delta % CLOCKS_PER_HOUR;
  sData.Minutes = delta / CLOCKS_PER_MIN;
  delta = delta % CLOCKS_PER_MIN;
  sData.Seconds = (float)delta / CLOCKS_PER_SEC;
  sData.Milliseconds = delta % CLOCKS_PER_SEC;
}

unsigned int StopWatch::GetHours() const { return sData.Hours; }

unsigned int StopWatch::GetMinutes() const { return sData.Minutes; }

float StopWatch::GetSeconds() const { return sData.Seconds; }

unsigned int StopWatch::GetMilliseconds() const { return sData.Milliseconds; }

std::string StopWatch::GetElapsedTime() {
  if (mStarted) {
  }
  return sElapsedTime;
}
// std::string StopWatch::GetCurrentElapsedTime() {
//     if (mStarted) {
//
//     }
//     return sElapsedTime;
// }

void StopWatch::CalculateElapsedTime() {
  char hour[4], min[4], sec[4], ms[4];

  sElapsedTime.clear();
  if (GetHours() > 0) {
    _itoa_s((int)GetHours(), hour, 10);
    sElapsedTime = *hour;
    sElapsedTime += "h ";
  }
  if (GetMinutes() > 0 || GetHours() > 0) {
    _itoa_s((int)GetMinutes(), min, 10);
    sElapsedTime += *min;
    sElapsedTime += "m ";
  }
  if (GetSeconds() > 0 || GetMinutes() > 0) {
    _itoa_s((int)GetSeconds(), sec, 10);
    sElapsedTime += *sec;
    sElapsedTime += "s ";
  }

  _itoa_s((long)GetMilliseconds(), ms, 10);
  sElapsedTime += *ms;
  sElapsedTime += "ms";
}

} // namespace RESANA