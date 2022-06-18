#include "SafeLockContainer.h"

namespace RESANA
{
	SafeLockContainer::SafeLockContainer()
	{
		mReadLock = std::shared_lock<std::shared_mutex>(mSmutex, std::defer_lock);
		mWriteLock = std::unique_lock<std::recursive_mutex>(mRmutex, std::defer_lock);
	}

	SafeLockContainer::~SafeLockContainer() = default;

	std::shared_mutex& SafeLockContainer::GetSharedMutex()
	{
		return mSmutex;
	}

	std::recursive_mutex& SafeLockContainer::GetMutex()
	{
		return mRmutex;
	}

	std::shared_lock<std::shared_mutex>& SafeLockContainer::GetReadLock()
	{
		return mReadLock;
	}

	std::unique_lock<std::recursive_mutex>& SafeLockContainer::GetWriteLock()
	{
		return mWriteLock;
	}

	void SafeLockContainer::NotifyOne()
	{
		mCondVar.notify_one();
	}

	void SafeLockContainer::NotifyAll()
	{
		mCondVar.notify_all();
	}

}