#include "slice.hpp"
#include <iostream>
#include <iomanip>

// A macro named DIM_SIZES will be compiled in.

// For the time being, this will serve as a testing file

template<unsigned ... ds>
void testBase()
{
	std::array<unsigned,sizeof...(ds)> dims {{ds...}};
	
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
}

template<bool prune, unsigned ... ds>
void test()
{
	using graph_t = slice_graph<prune,unsigned,ds...>;
	
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

template<unsigned ... ds>
void testPruned()
{
	test<true,ds...>();
}

template<unsigned ... ds>
void testUnpruned()
{
	test<false,ds...>();
}

int main()
{
	std::cout << "Base tests:\n";
	testBase<DIM_SIZES>();
	
	std::cout << "\nUnpruned tests:\n";
	testUnpruned<DIM_SIZES>();
	
	std::cout << "\nPruned tests:\n";
	testPruned<DIM_SIZES>();
}
