#include "subtree.hpp"

#include "doctest/doctest.h"

TEST_CASE("dimensions: {1}")
{
	static_hrp_graph<1> s_graph;
	hrp_graph r_graph {1};
	
	subtree s_subtree(s_graph, 0);
	subtree r_subtree(r_graph, 0);
	
	CHECK(s_subtree.n_induced() == 1);
	CHECK(r_subtree.n_induced() == 1);
	
	CHECK(s_subtree.has(0));
	CHECK(r_subtree.has(0));
	
	CHECK(s_subtree.cnt(0) == 0);
	CHECK(r_subtree.cnt(0) == 0);
}

TEST_CASE("dimensions: {2}")
{
	static_hrp_graph<2> s_graph;
	hrp_graph r_graph {2};
	
	subtree s_subtree(s_graph, 0);
	subtree r_subtree(r_graph, 0);
	
	CHECK(s_subtree.n_induced() == 1);
	CHECK(r_subtree.n_induced() == 1);
	
	CHECK(s_subtree.has(0));
	CHECK(r_subtree.has(0));
	
	CHECK(s_subtree.cnt(0) == 0);
	CHECK(r_subtree.cnt(0) == 0);
	
	CHECK(!s_subtree.has(1));
	CHECK(!r_subtree.has(1));
	
	CHECK(s_subtree.cnt(1) == 1);
	CHECK(r_subtree.cnt(1) == 1);
	
	s_subtree.add(1);
	r_subtree.add(1);
	
	CHECK(s_subtree.n_induced() == 2);
	CHECK(r_subtree.n_induced() == 2);
	
	CHECK(s_subtree.has(0));
	CHECK(r_subtree.has(0));
	
	CHECK(s_subtree.cnt(0) == 1);
	CHECK(r_subtree.cnt(0) == 1);
	
	CHECK(s_subtree.has(1));
	CHECK(r_subtree.has(1));
	
	CHECK(s_subtree.cnt(1) == 1);
	CHECK(r_subtree.cnt(1) == 1);
	
	s_subtree.rem(1);
	r_subtree.rem(1);
	
	CHECK(s_subtree.n_induced() == 1);
	CHECK(r_subtree.n_induced() == 1);
	
	CHECK(s_subtree.has(0));
	CHECK(r_subtree.has(0));
	
	CHECK(s_subtree.cnt(0) == 0);
	CHECK(r_subtree.cnt(0) == 0);
	
	CHECK(!s_subtree.has(1));
	CHECK(!r_subtree.has(1));
	
	CHECK(s_subtree.cnt(1) == 1);
	CHECK(r_subtree.cnt(1) == 1);
}
