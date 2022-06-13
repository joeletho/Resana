#pragma once

#include "SafeLockContainer.h"

namespace RESANA 
{

	class ConcurrentProcess
	{
	public:
		ConcurrentProcess(std::string name);
		virtual ~ConcurrentProcess() {};

	protected:

		//virtual void Run() {};
		//virtual void Terminate() {};

		SafeLockContainer& GetLockContainer() { return mLockContainer; }

	private:
		std::string mDebugName{};

		SafeLockContainer mLockContainer{};
	};

}