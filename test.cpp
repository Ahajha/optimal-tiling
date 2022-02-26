#include "static_graph.hpp"
#include <array>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

template<class container_t, class graph_t>
void compare_neighbors(const container_t& neighbors, const graph_t& graph)
{
	CHECK(neighbors.size() == graph.vertices.size());
	
	for (std::size_t i = 0; i < neighbors.size(); ++i)
	{
		CHECK(neighbors[i].size() == graph.vertices[i].neighbors.size());
		
		for (std::size_t j = 0; j < neighbors[i].size(); ++j)
		{
			CHECK(neighbors[i][j] ==
				static_cast<std::size_t>(graph.vertices[i].neighbors[j]));
		}
	}
}

template<class container_t, class graph_t>
void compare_directions(const container_t& directions, const graph_t& graph)
{
	CHECK(directions.size() == graph.vertices.size());
	
	for (std::size_t i = 0; i < directions.size(); ++i)
	{
		CHECK(directions[i].size() == graph.vertices[i].directions.size());
		
		for (std::size_t j = 0; j < directions[i].size(); ++j)
		{
			CHECK(directions[i][j] ==
				static_cast<std::size_t>(graph.vertices[i].directions[j]));
		}
	}
}

TEST_CASE("dimensions: {1}")
{
	static_hrp_graph<1> graph;
	
	CHECK(std::same_as<decltype(graph)::vertex_id, std::uint8_t>);
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
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
	static_hrp_graph<1,1> graph;
	
	CHECK(graph.vertices.size() == 1);
}

TEST_CASE("dimensions: {1,1,1}")
{
	static_hrp_graph<1,1,1> graph;
	
	CHECK(graph.vertices.size() == 1);
}

TEST_CASE("dimensions: {2}")
{
	static_hrp_graph<2> graph;
	
	CHECK(graph.vertices.size() == 2);
}

TEST_CASE("dimensions: {2,2}")
{
	static_hrp_graph<2,2> graph;
	
	CHECK(graph.vertices.size() == 4);
}

TEST_CASE("dimensions: {2,2,2}")
{
	static_hrp_graph<2,2,2> graph;
	
	CHECK(graph.vertices.size() == 8);
}

TEST_CASE("dimensions: {2,3}")
{
	static_hrp_graph<2,3> graph;
	
	CHECK(graph.vertices.size() == 6);
}

TEST_CASE("dimensions: {3,2,3}")
{
	static_hrp_graph<3,2,3> graph;
	
	CHECK(graph.vertices.size() == 18);
}

TEST_CASE("dimensions: {4,1,2,3}")
{
	static_hrp_graph<4,1,2,3> graph;
	
	CHECK(graph.vertices.size() == 24);
}
