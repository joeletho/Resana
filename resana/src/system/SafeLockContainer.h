#pragma once

#include <condition_variable>
#include <shared_mutex>

namespace RESANA {

class SafeLockContainer {
public:
  SafeLockContainer();
  ~SafeLockContainer();

  std::recursive_mutex &GetMutex();
  std::shared_mutex &GetSharedMutex();
  std::shared_lock<std::shared_mutex> &GetReadLock();
  std::unique_lock<std::recursive_mutex> &GetWriteLock();

  void NotifyOne();
  void NotifyAll();

  template <class Lock> void Wait(Lock &lock);

  template <class Lock> void Wait(Lock &lock, bool predicate);

  template <class Lock> void WaitFor(Lock &lock, uint32_t waitTime);
  template <class Lock>
  void WaitFor(Lock &lock, std::chrono::milliseconds waitTime);
  template <class Lock>
  void WaitFor(Lock &lock, std::chrono::milliseconds waitTime, bool predicate);
  template <class Lock>
  void WaitFor(Lock &lock, uint32_t waitTime, bool predicate);

private:
  std::unique_lock<std::recursive_mutex> mWriteLock;
  std::shared_lock<std::shared_mutex> mReadLock;

  std::shared_mutex mSharedMutex{};
  std::recursive_mutex mRecursiveMutex{};
  std::condition_variable_any mCondVar{};
};

template <class Lock> void SafeLockContainer::Wait(Lock &lock) {
  mCondVar.wait(lock);
}

template <class Lock> void SafeLockContainer::Wait(Lock &lock, bool predicate) {
  mCondVar.wait(lock, [&predicate] { return predicate; });
}

template <class Lock>
void SafeLockContainer::WaitFor(Lock &lock, std::chrono::milliseconds waitTime,
                                bool predicate) {
  mCondVar.wait_for(lock, waitTime, [&predicate] { return predicate; });
}

template <class Lock>
void SafeLockContainer::WaitFor(Lock &lock, uint32_t waitTime, bool predicate) {
  mCondVar.wait_for(lock, std::chrono::milliseconds(waitTime),
                    [&predicate] { return predicate; });
}

template <class Lock>
void SafeLockContainer::WaitFor(Lock &lock,
                                std::chrono::milliseconds waitTime) {
  mCondVar.wait_for(lock, waitTime);
}

template <class Lock>
void SafeLockContainer::WaitFor(Lock &lock, uint32_t waitTime) {
  mCondVar.wait_for(lock, std::chrono::milliseconds(waitTime));
}

} // namespace RESANA
