#include "subtree.hpp"

#include "doctest/doctest.h"

TEST_CASE("dimensions: {1}")
{
	static_hrp_graph<1> s_graph;
	hrp_graph r_graph {1};
	
	subtree s_subtree(s_graph, 0);
	subtree r_subtree(r_graph, 0);
	
	CHECK(s_subtree.has(0));
	CHECK(r_subtree.has(0));
	
	CHECK(s_subtree.cnt(0) == 0);
	CHECK(r_subtree.cnt(0) == 0);
}
