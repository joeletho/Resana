#pragma once

#include <vector>
#include <deque>

namespace RESANA {

	template<typename T>
	void Clear(std::vector<T>& vec)
	{
		vec.erase(vec.begin(), vec.end());
	}

	template<typename T>
	void Free(std::vector<T>& vec)
	{
		for (T ptr : vec)
		{
			if (ptr != nullptr)
			{
				ptr = nullptr;
				free(ptr);
			}
		}
	}

	template<typename T>
	void Delete(std::vector<T>& vec)
	{
		for (T ptr : vec) { delete ptr; }
	}

	template<typename T>
	void Copy(std::vector<T>& lhs, const std::vector<T>& rhs)
	{
		Clear(lhs);
		lhs.reserve(rhs.size());
		std::unique_copy(rhs.begin(), rhs.end(), std::back_inserter(lhs));
	}

	template<typename T>
	void Copy(std::vector<std::initializer_list<T>>& lhs, const std::vector<T>& rhs)
	{
		Clear(lhs);
		lhs.reserve(rhs.size());
		std::unique_copy(rhs.begin(), rhs.end(), std::back_inserter(lhs));
	}

	template<typename T>
	void CopyProtected(std::vector<T>& lhs, const std::vector<T>& rhs, std::mutex& lhs_mtx, std::mutex& rhs_mtx, bool del_lhs)
	{
		{
			static std::mutex funcMutex;
			std::lock_guard<std::mutex> lock(funcMutex);
			{
				std::lock<std::mutex>(lhs_mtx, rhs_mtx);
				std::lock_guard<std::mutex> lock1(lhs_mtx, std::adopt_lock);
				std::lock_guard<std::mutex> lock2(rhs_mtx, std::adopt_lock);

				if (del_lhs) { Delete(lhs); }
				Copy(lhs, rhs);
			}
		}
	}

	template<class C, typename T = typename C::value_type>
	T CalculateAverage(C& values)
	{
		T sum = 0;
		for (const auto val : values) { sum += val; }

		return sum / values.size();
	}

} // RESANA
