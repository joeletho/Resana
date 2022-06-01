#pragma once

#include <mutex>
#include <condition_variable>

namespace RESANA {

	class ConcurrentProcess {
	public:
		ConcurrentProcess(std::string name);
		virtual ~ConcurrentProcess() {};

		virtual void Start() {};
		virtual void Stop() {};

	protected:
		void NotifyOne();
		void NotifyAll();

		// TODO: Get this working
		//void Wait(std::unique_lock<std::mutex>& lock, bool condition);
		//void WaitFor(std::unique_lock<std::mutex>& lock, unsigned int milliseconds, bool condition);

		//inline std::condition_variable& GetCV() { return mProcCondition; };

	private:
		std::string mDebugName;
		std::condition_variable mProcCondition;
	};

}