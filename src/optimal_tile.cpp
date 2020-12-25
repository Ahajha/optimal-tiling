#include "slice.hpp"
#include <iostream>
#include <iomanip>

// A macro named DIM_SIZES will be compiled in.

// For the time being, this will serve as a testing file

int main()
{
	auto dims = variadic_array<unsigned,DIM_SIZES>{};
	
	for (auto i : dims)
	{
		std::cout << i << ' ';
	}
	std::cout << '\n';
	
	for (const auto& perm : permutationSet<unsigned,DIM_SIZES>::perms)
	{
		for (auto i : perm)
		{
			std::cout << std::setw(2) << i << ' ';
		}
		std::cout << '\n';
	}
	
	slice_graph<true,unsigned,DIM_SIZES>::enumerate();
}
