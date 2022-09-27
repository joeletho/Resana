#include "LogicalCoreData.h"
#include "rspch.h"

namespace RESANA {
LogicalCoreData::LogicalCoreData() = default;

LogicalCoreData::LogicalCoreData(const LogicalCoreData& other)
{
    Copy(other);
}

LogicalCoreData::~LogicalCoreData() = default;

std::mutex& LogicalCoreData::GetMutex()
{
    return mMutex;
}

std::vector<std::shared_ptr<PdhItem>>& LogicalCoreData::GetProcessors()
{
    return mProcessors;
}

void LogicalCoreData::SetProcessorRef(PdhItem* ref)
{
    if (mProcessorRef) {
        delete mProcessorRef;
        mProcessorRef = nullptr;
    }
    mProcessorRef = ref;
}

PdhItem* LogicalCoreData::GetProcessorRef() const
{
    return mProcessorRef;
}

DWORD& LogicalCoreData::GetSize()
{
    return mSize;
}

DWORD& LogicalCoreData::GetBuffer()
{
    return mBuffer;
}

void LogicalCoreData::Clear()
{
    std::mutex mutex;
    std::scoped_lock lock(mutex);
    mProcessors.clear();
}

void LogicalCoreData::Copy(const LogicalCoreData& other)
{
    if (this == &other) {
        return;
    }

    Clear();

    std::scoped_lock lock(mMutex);

    for (const auto& processor : other.mProcessors) {
        auto copy = std::make_shared<PdhItem>(*processor);
        mProcessors.emplace_back(copy);
    }

    mProcessorRef = other.mProcessorRef;
    mSize = other.mSize;
    mBuffer = other.mBuffer;
}

LogicalCoreData& LogicalCoreData::operator=(const LogicalCoreData& rhs)
{
    if (this != &rhs) {
        Copy(rhs);
    }
    return *this;
}
}