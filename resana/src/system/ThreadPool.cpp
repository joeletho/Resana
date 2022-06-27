#include "ThreadPool.h"

#include "helpers/Time.h"

namespace RESANA
{
	ThreadPool::ThreadPool()
	{
	}

	ThreadPool::~ThreadPool()
	{
	}

	void ThreadPool::Start()
	{
		const uint32_t numThreads = std::thread::hardware_concurrency(); // Max # of threads the system supports
		mThreads.resize(numThreads);

		for (uint32_t i = 0; i < numThreads; i++) {
			mThreads.at(i) = std::thread([this] {ThreadLoop();});
		}
	}

	void ThreadPool::Stop()
	{
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mShouldTerminate = true;
		}
		mCondition.notify_all();

		//Time::Sleep(1000);

		for (std::thread& active_thread : mThreads) {
			active_thread.join();
		}
		mThreads.clear();
	}

	void ThreadPool::Queue(const std::function<void()>& job)
	{
		{
			std::unique_lock<std::mutex> lock(mMutex);
			mQueue.push(job);
		}
		mCondition.notify_one();
	}

	bool ThreadPool::Busy()
	{
		bool pollBusy;
		{
			std::unique_lock<std::mutex> lock(mMutex);
			pollBusy = mQueue.empty();
		}
		return pollBusy;
	}

	void ThreadPool::ThreadLoop()
	{
		while (true)
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(mMutex);
				mCondition.wait(lock, [&,this] {
					return !mQueue.empty() || mShouldTerminate;
					});

				if (mShouldTerminate) {
					return;
				}

				job = mQueue.front();
				mQueue.pop();
			}
			job();
		}
	}
}
