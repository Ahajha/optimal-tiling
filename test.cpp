#include "graph.hpp"
#include "static_graph.hpp"
#include <array>
#include <vector>
#include <algorithm>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

// Used to unify no_vertex between static and runtime versions
constexpr std::size_t global_no_vertex = std::numeric_limits<std::size_t>::max();

template<class container_t, class graph_t>
void compare_neighbors(const container_t& neighbors, const graph_t& graph)
{
	REQUIRE(neighbors.size() == graph.vertices.size());
	
	for (std::size_t i = 0; i < neighbors.size(); ++i)
	{
		CHECK(neighbors.at(i).size() == graph.vertices.at(i).neighbors.size());
		
		CHECK(std::ranges::is_sorted(graph.vertices.at(i).neighbors));
		
		for (std::size_t j = 0; j < neighbors.at(i).size(); ++j)
		{
			CHECK(neighbors.at(i).at(j) ==
				static_cast<std::size_t>(graph.vertices.at(i).neighbors.at(j)));
		}
	}
}

template<class container_t, class graph_t>
void compare_directions(const container_t& directions, const graph_t& graph)
{
	REQUIRE(directions.size() == graph.vertices.size());
	
	for (std::size_t i = 0; i < directions.size(); ++i)
	{
		CHECK(directions.at(i).size() == graph.vertices.at(i).directions.size());
		
		for (std::size_t j = 0; j < directions.at(i).size(); ++j)
		{
			// Different graphs define no_vertex differently, so if the graph
			// matches its own no_vertex, check if directions matches
			// global_no_vertex. Otherwise, compare normally.
			if (graph.vertices.at(i).directions.at(j) == graph.no_vertex)
			{
				CHECK(directions.at(i).at(j) == global_no_vertex);
			}
			else
			{
				CHECK(directions.at(i).at(j) ==
					static_cast<std::size_t>(graph.vertices.at(i).directions.at(j)));
			}
		}
	}
}

TEST_CASE("dimensions: {1}")
{
	static_hrp_graph<1> s_graph;
	hrp_graph r_graph {1};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {1,1}")
{
	static_hrp_graph<1,1> s_graph;
	hrp_graph r_graph {1,1};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex,
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {1,1,1}")
{
	static_hrp_graph<1,1,1> s_graph;
	hrp_graph r_graph {1,1,1};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex,
			global_no_vertex,
			global_no_vertex,
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {2}")
{
	static_hrp_graph<2> s_graph;
	hrp_graph r_graph {2};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1},
		{0}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			1
		},
		{
			0,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {2,2}")
{
	static_hrp_graph<2,2> s_graph;
	hrp_graph r_graph {2,2};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 2},
		{0, 3},
		{0, 3},
		{1, 2}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex,
			1,
			2
		},
		{
			global_no_vertex,
			0,
			global_no_vertex,
			3
		},
		{
			0,
			global_no_vertex,
			3,
			global_no_vertex
		},
		{
			1,
			2,
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {3,2}")
{
	static_hrp_graph<3,2> s_graph;
	hrp_graph r_graph {3,2};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 3},
		{0, 2, 4},
		{1, 5},
		{0, 4},
		{1, 3, 5},
		{2, 4}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex,
			1,
			3
		},
		{
			global_no_vertex,
			0,
			2,
			4
		},
		{
			global_no_vertex,
			1,
			global_no_vertex,
			5
		},
		{
			0,
			global_no_vertex,
			4,
			global_no_vertex
		},
		{
			1,
			3,
			5,
			global_no_vertex
		},
		{
			2,
			4,
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}

TEST_CASE("dimensions: {3,2,1}")
{
	static_hrp_graph<3,2,1> s_graph;
	hrp_graph r_graph {3,2,1};
	
	std::vector<std::vector<std::size_t>> neighbors
	{
		{1, 3},
		{0, 2, 4},
		{1, 5},
		{0, 4},
		{1, 3, 5},
		{2, 4}
	};
	
	compare_neighbors(neighbors, s_graph);
	compare_neighbors(neighbors, r_graph);
	
	std::vector<std::vector<std::size_t>> directions
	{
		{
			global_no_vertex,
			global_no_vertex,
			global_no_vertex,
			1,
			3,
			global_no_vertex
		},
		{
			global_no_vertex,
			global_no_vertex,
			0,
			2,
			4,
			global_no_vertex
		},
		{
			global_no_vertex,
			global_no_vertex,
			1,
			global_no_vertex,
			5,
			global_no_vertex
		},
		{
			global_no_vertex,
			0,
			global_no_vertex,
			4,
			global_no_vertex,
			global_no_vertex
		},
		{
			global_no_vertex,
			1,
			3,
			5,
			global_no_vertex,
			global_no_vertex
		},
		{
			global_no_vertex,
			2,
			4,
			global_no_vertex,
			global_no_vertex,
			global_no_vertex
		}
	};
	
	compare_directions(directions, s_graph);
	compare_directions(directions, r_graph);
}
