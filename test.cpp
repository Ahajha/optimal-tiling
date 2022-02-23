#include "static_graph.hpp"
#include <array>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

template<class container_t, class graph_t>
void compare_neighbors(const container_t& neighbors, const graph_t& graph)
{
	for (typename graph_t::vertex_id i = 0; i < graph.n_vertices; ++i)
	{
		CHECK(neighbors[i].size() == graph.vertices[i].neighbors.size());
		
		for (std::size_t j = 0; j < neighbors[i].size(); ++j)
		{
			CHECK(neighbors[i][j] == graph.vertices[i].neighbors[j]);
		}
	}
}

template<class container_t, class graph_t>
void compare_directions(const container_t& directions, const graph_t& graph)
{
	for (typename graph_t::vertex_id i = 0; i < graph.n_vertices; ++i)
	{
		CHECK(directions[i].size() == graph.vertices[i].directions.size());
		
		for (std::size_t j = 0; j < directions[i].size(); ++j)
		{
			CHECK(directions[i][j] == graph.vertices[i].directions[j]);
		}
	}
}

TEST_CASE("dimensions: {1}")
{
	static_hrp_graph<1> graph;
	
	CHECK(graph.n_vertices == 1);
	CHECK(std::same_as<decltype(graph)::vertex_id, std::uint8_t>);
	
	std::array<std::vector<decltype(graph)::vertex_id>, graph.n_vertices> neighbors
	{
		{
			{}
		}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::array<std::array<decltype(graph)::vertex_id, 2>, graph.n_vertices> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
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
