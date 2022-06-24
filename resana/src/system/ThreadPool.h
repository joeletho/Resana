#pragma once

#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>
#include <vector>
#include <thread>

namespace RESANA
{

	class ThreadPool
	{
	public:
		ThreadPool();
		~ThreadPool();

		void Start();
		void Stop();

		void Queue(const std::function<void()>& job);
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

}