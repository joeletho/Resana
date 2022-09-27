#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace RESANA {

class ThreadPool {
public:
  ThreadPool();
  ~ThreadPool();

  void Start();
  void Stop();

  void Queue(const std::function<void()> &job);
  bool Busy();

private:
  void ThreadLoop();

private:
  std::queue<std::function<void()>> mQueue{};
  std::vector<std::thread> mThreads{};

  std::mutex mMutex{};
  std::condition_variable mCondition{};

  bool mShouldTerminate = false;
};

} // namespace RESANA