#include "rspch.h"
#include "ProcessorData.h"

namespace RESANA
{
	ProcessorData::ProcessorData() = default;

	ProcessorData::ProcessorData(const ProcessorData* other)
	{
	}

	ProcessorData::~ProcessorData() {
		std::scoped_lock lock(mMutex);
	}

	std::mutex& ProcessorData::GetMutex()
	{
		return mMutex;
	}

	std::vector<std::shared_ptr<PdhItem>>& ProcessorData::GetProcessors()
	{
		return mProcessors;
	}

	void ProcessorData::SetProcessorRef(PdhItem* ref)
	{
		mProcessorRef = ref;
	}

	PdhItem* ProcessorData::GetProcessorRef()
	{
		return mProcessorRef;
	}

	DWORD& ProcessorData::GetSize()
	{
		return mSize;
	}

	DWORD& ProcessorData::GetBuffer()
	{
		return mBuffer;
	}

	void ProcessorData::Clear()
	{
		std::scoped_lock lock(mMutex);
		mProcessors.clear();
	}

	ProcessorData& ProcessorData::operator=(ProcessorData* rhs)
	{
		if (this == rhs) { return *this; }

		Clear();

		std::scoped_lock lock(mMutex);
		for (const auto& p : rhs->mProcessors) {
			mProcessors.push_back(std::make_shared<PdhItem>(*p));
		}

		mProcessorRef = rhs->GetProcessorRef();
		mSize = rhs->GetSize();
		mBuffer = rhs->GetBuffer();

		return *this;
	}
}