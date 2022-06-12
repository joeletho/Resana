#include "SafeLockContainer.h"

namespace RESANA
{
	SafeLockContainer::SafeLockContainer()
	{
		mReadLock = std::shared_lock<std::shared_mutex>(mMutex , std::defer_lock);
		mWriteLock = std::unique_lock<std::shared_mutex>(mMutex, std::defer_lock);
	}

	SafeLockContainer::~SafeLockContainer() {};

	std::shared_mutex& SafeLockContainer::GetMutex()
	{
		return mMutex;
	}

	std::shared_lock<std::shared_mutex>& SafeLockContainer::GetReadLock()
	{
		return mReadLock;
	}

	std::unique_lock<std::shared_mutex>& SafeLockContainer::GetWriteLock()
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