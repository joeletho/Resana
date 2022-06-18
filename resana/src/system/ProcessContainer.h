#pragma once

#include <vector>
#include <mutex>

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

		void AddEntry(ProcessEntry* entry);
		void SelectEntry(ProcessEntry* entry);
		void EraseEntry(const ProcessEntry* entry);
		void Clear();

	private:
		std::mutex mMutex{};
		std::vector<ProcessEntry*> mEntries{};
		
		ProcessEntry* mSelectedEntry = nullptr;
	};

}
