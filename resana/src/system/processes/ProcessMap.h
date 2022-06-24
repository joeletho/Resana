#pragma once

#include "ProcessEntry.h"

#include <map>
#include <mutex>

namespace RESANA
{

	class ProcessMap
	{
		typedef unsigned long ulong;
	public:
		ProcessMap();
		~ProcessMap();

		std::recursive_mutex& GetMutex();

		[[nodiscard]] ProcessEntry* Find(ulong procId) const;

		[[nodiscard]] int Count(ulong procId)const;
		[[nodiscard]] int Size() const;

		[[nodiscard]] bool Contains(ulong procId) const;
		[[nodiscard]] bool Empty() const;

		void Clear();
		void Emplace(ProcessEntry* entry);
		void Erase(ulong procId);
		void Erase(const ProcessEntry* entry);

		// Overloads
		auto begin() { return mMap.begin(); }
		auto end() { return mMap.end(); }
		[[nodiscard]] auto cbegin() const { return mMap.cbegin(); }
		[[nodiscard]] auto cend() const { return mMap.cend(); }

		ProcessEntry* operator[](ulong procId) const;

	private:
		std::map<ulong, ProcessEntry*> mMap{};
		std::recursive_mutex mMutex{};
		std::unique_lock<std::recursive_mutex> mLock;
	};

}