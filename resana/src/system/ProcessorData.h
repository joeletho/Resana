#pragma once

#include <vector>
#include <memory>
#include <mutex>

#include <tchar.h>
#include <Pdh.h>

namespace RESANA
{

	typedef PDH_FMT_COUNTERVALUE_ITEM PdhItem;

	struct PDHCounter
	{
		HANDLE Handle{};
		HQUERY Query{};
		PDH_HCOUNTER Counter{};
		ULARGE_INTEGER Last{};
		ULARGE_INTEGER LastSys{};
		ULARGE_INTEGER LastUser{};
	};


	class ProcessorData
	{
	public:
		ProcessorData();
		explicit ProcessorData(const ProcessorData* other);
		~ProcessorData();

		std::mutex& GetMutex();

		std::vector<std::shared_ptr<PdhItem>>& GetProcessors();

		void SetProcessorRef(PdhItem* ref);
		PdhItem* GetProcessorRef();

		[[nodiscard]] DWORD& GetSize();

		DWORD& GetBuffer();


		void Clear();

		ProcessorData& operator=(ProcessorData* rhs);

	private:
		std::mutex mMutex{};
		std::vector<std::shared_ptr<PdhItem>> mProcessors{};
		PdhItem* mProcessorRef = nullptr;
		DWORD mSize = 0;
		DWORD mBuffer = 0;
	};

}
