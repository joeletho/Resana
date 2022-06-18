#include "rspch.h"
#include "ProcessContainer.h"

#include "ProcessEntry.h"

namespace RESANA
{

	ProcessContainer::ProcessContainer() = default;

	ProcessContainer::ProcessContainer(const ProcessContainer* other)
		:mEntries(other->mEntries) {}


	ProcessContainer::~ProcessContainer()
	{
		Clear();
	};

	int ProcessContainer::GetNumEntries() const
	{
		return (int)mEntries.size();
	}

	std::mutex& ProcessContainer::GetMutex() {
		return mMutex;
	}

	std::vector<ProcessEntry*>& ProcessContainer::GetEntries()
	{
		return mEntries;
	}

	ProcessEntry* ProcessContainer::GetSelectedEntry() const
	{
		return mSelectedEntry;
	}

	void ProcessContainer::SelectEntry(ProcessEntry* entry)
	{
		if (!entry) { return; }

		if (entry->IsSelected())
		{
			entry->Deselect();
			mSelectedEntry = nullptr;
		}
		else
		{
			if (mSelectedEntry) {
				mSelectedEntry->Deselect();
			}

			entry->Select();
			mSelectedEntry = entry;
		}
	}

	void ProcessContainer::AddEntry(ProcessEntry* entry)
	{
		mEntries.emplace_back(entry);
	}

	void ProcessContainer::EraseEntry(const ProcessEntry* entry)
	{
		if (!entry) { return; }

		for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
		{
			if (auto proc =
				mEntries.at(reinterpret_cast<std::vector<ProcessEntry*>::size_type>(&it));
				proc->ProcessId() == entry->ProcessId())
			{
				delete proc;
				proc = nullptr;
				break;
			}
		}
	}

	void ProcessContainer::Clear()
	{
		std::scoped_lock slock(mMutex);
		for (auto entry : mEntries)
		{
			delete entry;
			entry = nullptr;
		}

		mEntries.clear();
	}

}
