#include "slice.hpp"
#include <iostream>
#include <iomanip>

// A macro named DIM_SIZES will be compiled in.

// For the time being, this will serve as a testing file

template<unsigned ... ds>
void test()
{
	auto dims = variadic_array<unsigned,ds...>{};
	
	std::cout << "Dimensions\n";
	for (auto i : dims)
	{
		std::cout << i << ' ';
	}
	std::cout << '\n';
	
	std::cout << "Permutations\n";
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
	
	std::cout << "Slices\n";
	for (const auto& s : graph_t::slices)
	{
		std::cout << s << '\n';
	}
	
	std::cout << "Graph\n";
	for (unsigned i = 0; i < graph_t::graph.size(); ++i)
	{
		std::cout << std::setw(3) << i << ":";
		for (unsigned adj : graph_t::graph[i].adjList)
		{
			std::cout << ' ' << adj;
		}
		std::cout << '\n';
	}
}

int main()
{
	test<DIM_SIZES>();
}
