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
		Clear(mMutex);
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

	ProcessEntry* ProcessContainer::FindEntry(const ProcessEntry* entry) const
	{
		if (!entry) { return nullptr; }
		return FindEntry(entry->GetProcessId());
	}

	ProcessEntry* ProcessContainer::FindEntry(uint32_t procId) const
	{
		for (auto* proc : mEntries)
		{
			if (proc->GetProcessId() == procId)
			{
				return proc;
			}
		}

		return nullptr;
	}

	void ProcessContainer::AddEntry(ProcessEntry* entry)
	{
		mEntries.emplace_back(entry);
	}

	void ProcessContainer::SelectEntry(const uint32_t procId, bool overwrite)
	{
		if (auto* entry = FindEntry(procId))
		{
			if (overwrite)
			{
				entry->Select();
				mSelectedEntry = entry;
			}
			else if (entry->IsSelected())
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
		else
		{
			mSelectedEntry = nullptr;
		}
	}

	void ProcessContainer::SelectEntry(ProcessEntry* entry, bool overwrite)
	{
		if (entry) {
			SelectEntry(entry->GetProcessId(), overwrite);
		} else
		{
			mSelectedEntry = nullptr;
		}
	}

	void ProcessContainer::EraseEntry(const ProcessEntry* entry)
	{
		if (!entry) { return; }

		for (auto it = mEntries.begin(); it != mEntries.end(); ++it)
		{
			if (auto proc =
				mEntries.at(reinterpret_cast<std::vector<ProcessEntry*>::size_type>(&it));
				proc->GetProcessId() == entry->GetProcessId())
			{
				delete proc;
				proc = nullptr;
				break;
			}
		}
	}

	void ProcessContainer::Copy(ProcessContainer* other)
	{
		if (this == other) { return; }

		Clear(mMutex);

		std::scoped_lock lock1(mMutex);
		std::scoped_lock lock2(other->GetMutex());

		for (const auto* entry : other->GetEntries()) {
			mEntries.emplace_back(new ProcessEntry(entry));
		}

		mSelectedEntry = other->GetSelectedEntry();
	}

}
