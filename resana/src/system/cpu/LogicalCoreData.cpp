#include "rspch.h"
#include "LogicalCoreData.h"

namespace RESANA
{
	LogicalCoreData::LogicalCoreData() = default;

	LogicalCoreData::LogicalCoreData(LogicalCoreData* other)
	{
		Copy(other);
	}

	LogicalCoreData::~LogicalCoreData() {
		std::scoped_lock lock(mMutex);
		for (const auto processor : mProcessors) {
			delete processor;
		}
	}

	std::mutex& LogicalCoreData::GetMutex()
	{
		return mMutex;
	}

	std::vector<PdhItem*>& LogicalCoreData::GetProcessors()
	{
		return mProcessors;
	}

	void LogicalCoreData::SetProcessorRef(PdhItem* ref)
	{
		mProcessorRef = ref;
	}

	PdhItem* LogicalCoreData::GetProcessorRef()
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
		std::scoped_lock lock(mMutex);
		mProcessors.clear();
	}

	void LogicalCoreData::Copy(LogicalCoreData* other)
	{
		if (this == other) { return; }

		Clear();

		std::scoped_lock lock(mMutex);

		for (const auto processor : other->mProcessors) {
			auto copy = new PdhItem(*processor);
			mProcessors.emplace_back(copy);
		}

		mProcessorRef = new PdhItem(*other->GetProcessorRef());
		mSize = other->GetSize();
		mBuffer = other->GetBuffer();

	}

	LogicalCoreData& LogicalCoreData::operator=(LogicalCoreData* rhs)
	{
		Copy(rhs);
		return *this;
	}
}