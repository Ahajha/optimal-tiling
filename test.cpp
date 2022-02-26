#include "static_graph.hpp"
#include <array>
#include <vector>
#include <algorithm>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

template<class container_t, class graph_t>
void compare_neighbors(const container_t& neighbors, const graph_t& graph)
{
	REQUIRE(neighbors.size() == graph.vertices.size());
	
	for (std::size_t i = 0; i < neighbors.size(); ++i)
	{
		CHECK(neighbors[i].size() == graph.vertices[i].neighbors.size());
		
		CHECK(std::ranges::is_sorted(graph.vertices[i].neighbors));
		
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
	REQUIRE(directions.size() == graph.vertices.size());
	
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
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}

TEST_CASE("dimensions: {1,1,1}")
{
	static_hrp_graph<1,1,1> graph;
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}

TEST_CASE("dimensions: {2}")
{
	static_hrp_graph<2> graph;
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1},
		{0}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			1
		},
		{
			0,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}

TEST_CASE("dimensions: {2,2}")
{
	static_hrp_graph<2,2> graph;
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 2},
		{0, 3},
		{0, 3},
		{1, 2}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex,
			1,
			2
		},
		{
			graph.no_vertex,
			0,
			graph.no_vertex,
			3
		},
		{
			0,
			graph.no_vertex,
			3,
			graph.no_vertex
		},
		{
			1,
			2,
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}

TEST_CASE("dimensions: {3,2}")
{
	static_hrp_graph<3,2> graph;
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 3},
		{0, 2, 4},
		{1, 5},
		{0, 4},
		{1, 3, 5},
		{2, 4}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex,
			1,
			3
		},
		{
			graph.no_vertex,
			0,
			2,
			4
		},
		{
			graph.no_vertex,
			1,
			graph.no_vertex,
			5
		},
		{
			0,
			graph.no_vertex,
			4,
			graph.no_vertex
		},
		{
			1,
			3,
			5,
			graph.no_vertex
		},
		{
			2,
			4,
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}

TEST_CASE("dimensions: {3,2,1}")
{
	static_hrp_graph<3,2,1> graph;
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 3},
		{0, 2, 4},
		{1, 5},
		{0, 4},
		{1, 3, 5},
		{2, 4}
	};
	
	compare_neighbors(neighbors, graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex,
			1,
			3,
			graph.no_vertex
		},
		{
			graph.no_vertex,
			graph.no_vertex,
			0,
			2,
			4,
			graph.no_vertex
		},
		{
			graph.no_vertex,
			graph.no_vertex,
			1,
			graph.no_vertex,
			5,
			graph.no_vertex
		},
		{
			graph.no_vertex,
			0,
			graph.no_vertex,
			4,
			graph.no_vertex,
			graph.no_vertex
		},
		{
			graph.no_vertex,
			1,
			3,
			5,
			graph.no_vertex,
			graph.no_vertex
		},
		{
			graph.no_vertex,
			2,
			4,
			graph.no_vertex,
			graph.no_vertex,
			graph.no_vertex
		}
	};
	
	compare_directions(directions, graph);
}
