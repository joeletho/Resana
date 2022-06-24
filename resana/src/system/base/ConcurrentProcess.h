#pragma once

#include "system/SafeLockContainer.h"

#include "system/ThreadPool.h"

namespace RESANA
{

	class ConcurrentProcess
	{
	public:
		explicit ConcurrentProcess(std::string name);
		virtual ~ConcurrentProcess() = default;

		std::recursive_mutex& GetMutex() { return mLockContainer.GetMutex(); }

	protected:

		//virtual void Run() {};
		//virtual void Terminate() {};

		SafeLockContainer& GetLockContainer() { return mLockContainer; }

	private:
		std::string mDebugName{};

		SafeLockContainer mLockContainer{};
	};

}