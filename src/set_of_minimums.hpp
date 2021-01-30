#ifndef SET_OF_MINIMUMS_HPP
#define SET_OF_MINIMUMS_HPP

#include <list>
#include <iostream>

template<class T, class compare = void>
class set_of_minimums
{
	std::list<T> vec;
	
	public:
	
	auto begin() const { return vec.begin(); }
	auto end() const { return vec.end(); }
	
	auto size() const { return vec.size(); }
	
	void insert(const T& v)
	{
		// Default value needed due to no default constructor.
		std::partial_ordering result = std::partial_ordering::unordered;
		
		// Find the first element that does not compare unordered to v.
		auto iter = std::find_if(vec.begin(), vec.end(),
			[&result, &v](const T& other)
		{
			if constexpr (std::is_same<compare,void>::value)
			{
				return (result = (v <=> other)) != std::partial_ordering::unordered;
			}
			else
			{
				return (result = compare{}(v, other)) != std::partial_ordering::unordered;
			}
		});
		
		// If there is no such element, add v to the set.
		if (iter == vec.end())
		{
			vec.emplace_back(v);
		}
		
		// If v is smaller than the given element, replace it
		else if (result == std::partial_ordering::less)
		{
			vec.erase(iter++);
			
			// Erase any other values that are larger than v.
			while (iter != vec.end())
			{
				if constexpr (std::is_same<compare,void>::value)
				{
					if (v < *iter)
					{
						vec.erase(iter++);
					}
					else
					{
						++iter;
					}
				}
				else
				{
					if (compare{}(v, *iter) == std::partial_ordering::less)
					{
						vec.erase(iter++);
					}
					else
					{
						++iter;
					}
				}
			}
			
			vec.emplace_back(v);
		}
		
		// Otherwise ignore v.
	}
};

#endif
