#include "SafeLockContainer.h"

namespace RESANA {
SafeLockContainer::SafeLockContainer() {
  mReadLock =
      std::shared_lock<std::shared_mutex>(mSharedMutex, std::defer_lock);
  mWriteLock =
      std::unique_lock<std::recursive_mutex>(mRecursiveMutex, std::defer_lock);
}

SafeLockContainer::~SafeLockContainer() = default;

std::shared_mutex &SafeLockContainer::GetSharedMutex() { return mSharedMutex; }

std::recursive_mutex &SafeLockContainer::GetMutex() { return mRecursiveMutex; }

std::shared_lock<std::shared_mutex> &SafeLockContainer::GetReadLock() {
  return mReadLock;
}

std::unique_lock<std::recursive_mutex> &SafeLockContainer::GetWriteLock() {
  return mWriteLock;
}

void SafeLockContainer::NotifyOne() { mCondVar.notify_one(); }

void SafeLockContainer::NotifyAll() { mCondVar.notify_all(); }

} // namespace RESANA