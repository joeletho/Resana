#include "rspch.h"
#include "ProcessMap.h"

namespace RESANA
{
	ProcessMap::ProcessMap()
		: mLock(mMutex, std::defer_lock)
	{
	}

	ProcessMap::~ProcessMap()
	{
		std::scoped_lock lock(mMutex);
		mMap.clear();
	}

	std::recursive_mutex& ProcessMap::GetMutex()
	{
		return mMutex;
	}

	void ProcessMap::Emplace(ProcessEntry* entry)
	{
		mMap.emplace(std::make_pair<>(entry->ProcessId(), entry));
	}

	void ProcessMap::Clear()
	{
		for (const auto& [id, entry] : mMap) {
			Erase(entry);
		}

	}

	ProcessEntry* ProcessMap::Find(const ulong procId) const
	{
		for (const auto& [currId, entry] : mMap)
		{
			if (currId == procId) {
				return entry;
			}
		}

		return nullptr;
	}

	bool ProcessMap::Contains(const ulong procId) const
	{
		if (auto* entry = Find(procId)) {
			return true;
		}

		return false;
	}

	bool ProcessMap::Empty() const
	{
		return mMap.empty();
	}

	int ProcessMap::Count(const ulong procId) const
	{
		if (auto* proc = Find(procId)) {
			return (int)mMap.count(procId);
		}
		return 0;
	}

	int ProcessMap::Size() const
	{
		return (int)mMap.size();
	}

	void ProcessMap::Erase(const ulong procId)
	{
		if (auto* proc = Find(procId))
		{
			mMap.erase(procId);
			delete proc;
			proc = nullptr;
		}
	}

	void ProcessMap::Erase(const ProcessEntry* entry)
	{
		if (!entry) { return; }
		if (auto* proc = Find(entry->ProcessId()))
		{
			if (proc != entry) // entry is a copy, delete both
			{
				delete entry;
				entry = nullptr;
			}

			mMap.erase(proc->ProcessId());

			delete proc;
			proc = nullptr;
		}
	}

	ProcessEntry* ProcessMap::operator[](const ulong procId) const
	{
		return Find(procId);
	}

}
