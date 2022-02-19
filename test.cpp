#include "static_graph.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("dimensions: {1}")
{
	using graph = static_hrp_graph<1>;
	
	CHECK(graph::n_vertices == 1);
}

TEST_CASE("dimensions: {1,1}")
{
	using graph = static_hrp_graph<1>;
	
	CHECK(graph::n_vertices == 1);
}

TEST_CASE("dimensions: {1,1,1}")
{
	using graph = static_hrp_graph<1,1,1>;
	
	CHECK(graph::n_vertices == 1);
}

TEST_CASE("dimensions: {2}")
{
	using graph = static_hrp_graph<2>;
	
	CHECK(graph::n_vertices == 2);
}

TEST_CASE("dimensions: {2,2}")
{
	using graph = static_hrp_graph<2,2>;
	
	CHECK(graph::n_vertices == 4);
}

TEST_CASE("dimensions: {2,2,2}")
{
	using graph = static_hrp_graph<2,2,2>;
	
	CHECK(graph::n_vertices == 8);
}

TEST_CASE("dimensions: {2,3}")
{
	using graph = static_hrp_graph<2,3>;
	
	CHECK(graph::n_vertices == 6);
}

TEST_CASE("dimensions: {3,2,3}")
{
	using graph = static_hrp_graph<3,2,3>;
	
	CHECK(graph::n_vertices == 18);
}

TEST_CASE("dimensions: {4,1,2,3}")
{
	using graph = static_hrp_graph<4,1,2,3>;
	
	CHECK(graph::n_vertices == 24);
}
