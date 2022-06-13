#pragma once

#include <shared_mutex>
#include <condition_variable>

namespace RESANA
{

	class SafeLockContainer
	{
	public:
		SafeLockContainer();
		~SafeLockContainer();

		std::shared_mutex& GetMutex();
		std::shared_lock<std::shared_mutex>& GetReadLock();
		std::unique_lock<std::shared_mutex>& GetWriteLock();

		void NotifyOne();
		void NotifyAll();

		template <class Lock>
		void Wait(Lock& lock);

		template <class Lock>
		void Wait(Lock& lock, bool predicate);

	private:
		std::shared_lock<std::shared_mutex> mReadLock;
		std::unique_lock<std::shared_mutex> mWriteLock;

		std::shared_mutex mMutex{};
		std::condition_variable_any mCondVar{};
	};

	template <class Lock>
	inline void SafeLockContainer::Wait(Lock& lock)
	{
		mCondVar.wait(lock);
	}

	template <class Lock>
	inline void SafeLockContainer::Wait(Lock& lock, bool predicate)
	{
		mCondVar.wait(lock, [&predicate] { return predicate; });
	}

}
