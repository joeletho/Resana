#pragma once

#include "system/SafeLockContainer.h"

#include "system/ThreadPool.h"

namespace RESANA {

class SystemObject {
public:
  explicit SystemObject(SystemObject *context) : mContext(context) {}
  virtual ~SystemObject() = default;

  virtual void Run() = 0;
  virtual void Stop() = 0;
  virtual void Shutdown() = 0;

protected:
  SafeLockContainer mLockContainer{};
  SystemObject *mContext = nullptr;
};

} // namespace RESANA