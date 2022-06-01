#include "ConcurrentProcess.h"

#include "helpers/Time.h"

namespace RESANA {

	ConcurrentProcess::ConcurrentProcess(std::string name)
		: mDebugName(std::move(name)) {}

	void ConcurrentProcess::NotifyOne()
	{
		mProcCondition.notify_one();
	}

	void ConcurrentProcess::NotifyAll()
	{
		mProcCondition.notify_all();
	}

	//void ConcurrentProcess::Wait(std::unique_lock<std::mutex>& lock, bool condition)
	//{
	//	mProcCondition.wait(lock, [condition]() { return condition; });
	//}

	//void ConcurrentProcess::WaitFor(std::unique_lock<std::mutex>& lock, unsigned int milliseconds, bool condition)
	//{
	//	while (!condition)
	//	{
	//		Sleep(milliseconds);
	//		mProcCondition.wait(lock, [condition]() { return condition; });
	//	}
	//}

}