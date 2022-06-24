#pragma once

#include <vector>
#include <mutex>

#include "ProcessEntry.h"

namespace RESANA
{
	class ProcessEntry;

	class ProcessContainer
	{
	public:
		ProcessContainer();
		explicit ProcessContainer(const ProcessContainer* other);
		~ProcessContainer();

		std::mutex& GetMutex();
		[[nodiscard]] int GetNumEntries() const;

		std::vector<ProcessEntry*>& GetEntries();
		[[nodiscard]] ProcessEntry* GetSelectedEntry() const;
		[[nodiscard]] ProcessEntry* FindEntry(const ProcessEntry* entry) const;
		[[nodiscard]] ProcessEntry* FindEntry(uint32_t procId) const;

		void AddEntry(ProcessEntry* entry);
		void SelectEntry(uint32_t procId, bool preserve = false);
		void SelectEntry(ProcessEntry* entry, bool preserve = false);
		void EraseEntry(const ProcessEntry* entry);
		void Copy(ProcessContainer* other);

		template <class _MutTy>
		void Clear(_MutTy& mutex);

	private:
		std::mutex mMutex{};
		std::vector<ProcessEntry*> mEntries{};

		ProcessEntry* mSelectedEntry = nullptr;
	};

	template <class _MutTy>
	void ProcessContainer::Clear(_MutTy& mutex)
	{
		std::scoped_lock slock(mutex);
		for (auto entry : mEntries)
		{
			delete entry;
			entry = nullptr;
		}

		mEntries.clear();
	}

}
