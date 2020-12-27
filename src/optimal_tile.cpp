#include "slice.hpp"
#include <iostream>
#include <iomanip>

// A macro named DIM_SIZES will be compiled in.

// For the time being, this will serve as a testing file

template<unsigned ... ds>
void test()
{
	auto dims = variadic_array<unsigned,ds...>{};
	
	for (auto i : dims)
	{
		std::cout << i << ' ';
	}
	std::cout << '\n';
	
	for (const auto& perm : permutationSet<unsigned,ds...>::perms)
	{
		for (auto i : perm)
		{
			std::cout << std::setw(2) << i << ' ';
		}
		std::cout << '\n';
	}
	
	using graph_t = slice_graph<false,unsigned,ds...>;
	
	graph_t::enumerate();
	
	for (const auto& s : graph_t::slices)
	{
		std::cout << s << '\n';
	}
}

int main()
{
	test<DIM_SIZES>();
}
