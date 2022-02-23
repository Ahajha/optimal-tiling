#pragma once 

#include "semiarray.hpp"
#include "min_fast_type.hpp"

#include <array>
#include <numeric>

/*
HRP = Hyper-Rectangular Prism

A static hrp graph is a hrp graph whose dimensions are known at compile time.
*/
template<std::size_t... dims>
	requires (sizeof...(dims) >= 1 && ((dims > 0) && ...))
class static_hrp_graph
{
	constexpr static std::array<std::size_t, sizeof...(dims)> dims_array{ dims... };
	
	public:
	
	constexpr static auto n_vertices = min_fast_type<(dims * ...)>::value;
	
	using vertex_id = std::remove_const_t<decltype(n_vertices)>;
	
	constexpr static vertex_id no_vertex = std::numeric_limits<vertex_id>::max();
	
	// Returns the number of vertices that would be in the graph if
	// it were truncated to a given number of dimensions Note that
	// if d == dim_array.size(), this is just the number of vertices.
	[[nodiscard]] constexpr static std::size_t size_of_dim(std::size_t d)
	{
		constexpr auto size_table = []
		{
			std::array<std::size_t, dims_array.size()> table;
			
			table.front() = 1;
			for (std::size_t i = 1; i < table.size(); ++i)
			{
				table[i] = table[i - 1] * dims_array[i];
			}
			
			return table;
		}();
		
		return size_table[d];
	}
	
	// Gets a specific dimension of the coordinate of c. Valid values of
	// d are 0 <= d < dim_array.size()
	[[nodiscard]] constexpr static vertex_id get_coord (std::size_t d, vertex_id vid)
	{
		return (vid / size_of_dim(d)) % dims_array[d];
	}
	
	// Returns the vertex id of the vertex "forward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] constexpr static vertex_id forward   (std::size_t d, vertex_id vid)
	{
		return (get_coord(d,vid) == dims_array[d] - 1)
			? no_vertex : vid + size_of_dim(d);
	}

	// Returns the vertex id of the vertex "backward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] constexpr static vertex_id backward  (std::size_t d, vertex_id vid)
	{
		return (get_coord(d,vid) == 0)
			? no_vertex : vid - size_of_dim(d);
	}
	
	struct vertex
	{
		// Neighbors are the vertices it actually
		// touches, directions are indexed according to
		// the direction enum below, EMPTY means there
		// is no vertex in a given direction.
		
		semiarray <vertex_id, dims_array.size() * 2> neighbors;
		std::array<vertex_id, dims_array.size() * 2> directions;
		
		constexpr vertex() = default;
		[[nodiscard]] constexpr vertex(vertex_id vid)
		{
			// The highest dimension has the largest and smallest neighbors.
			// The second highest dimension has the second largest
			// and second smallest neighbors, etc.
			for (unsigned d = 0; d < dims_array.size(); ++d)
			{
				directions[dims_array.size() - d - 1] = backward(d, vid);
				directions[dims_array.size() + d    ] = forward (d, vid);
			}
			
			// Neighbors need to be in ascending order of ID
			for (vertex_id n : directions)
			{
				if (n != no_vertex)
				{
					neighbors.push_back(n);
				}
			}
		}
	};
	
	std::array<vertex, n_vertices> vertices;
	
	[[nodiscard]] constexpr static_hrp_graph()
	{
		for (vertex_id i = 0; i < vertices.size(); ++i)
		{
			vertices[i] = vertex(i);
		}
	} 
	
	// Returns true iff vid is an element on the outer shell of the hypercube.
	[[nodiscard]] constexpr bool is_on_outer_shell(vertex_id vid) const
	{
		return vertices[vid].neighbors.size() != dims_array.size() * 2;
	}
};
