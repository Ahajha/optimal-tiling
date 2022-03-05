#pragma once 

#include <vector>
#include <numeric>
#include <cstdint>

/*
HRP = Hyper-Rectangular Prism
*/
class hrp_graph
{
	public:
	using vertex_id = std::uint32_t;
	
	constexpr static vertex_id no_vertex = std::numeric_limits<vertex_id>::max();
	
	// Returns the number of vertices that would be in the graph if
	// it were truncated to a given number of dimensions Note that
	// if d == dim_array.size(), this is just the number of vertices.
	[[nodiscard]] vertex_id size_of_dim(std::size_t d) const
	{
		return dim_sizes[d];
	}
	
	// Gets a specific dimension of the coordinate of c. Valid values of
	// d are 0 <= d < dim_array.size()
	[[nodiscard]] vertex_id get_coord(std::size_t d, vertex_id vid) const
	{
		return (vid / size_of_dim(d)) % dims_array[d];
	}
	
	// Returns the vertex id of the vertex "forward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] vertex_id forward  (std::size_t d, vertex_id vid) const
	{
		return (get_coord(d,vid) == dims_array[d] - 1)
			? no_vertex : vid + size_of_dim(d);
	}

	// Returns the vertex id of the vertex "backward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] vertex_id backward (std::size_t d, vertex_id vid) const
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
		
		std::vector<vertex_id> neighbors;
		std::vector<vertex_id> directions;
		
		vertex() = default;
		
		private:
		
		[[nodiscard]] vertex(vertex_id vid, const hrp_graph& host_graph)
			: directions(host_graph.dims_array.size() * 2)
		{
			// The highest dimension has the largest and smallest neighbors.
			// The second highest dimension has the second largest
			// and second smallest neighbors, etc.
			//
			// Within the same dimension, the larger index is forwards.
			for (unsigned d = 0; d < host_graph.dims_array.size(); ++d)
			{
				directions[host_graph.dims_array.size() - d - 1] = host_graph.backward(d, vid);
				directions[host_graph.dims_array.size() + d    ] = host_graph.forward (d, vid);
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
		
		friend class hrp_graph;
	};
	
	[[nodiscard]] hrp_graph(std::initializer_list<vertex_id> dims)
		: dims_array(dims.begin(), dims.end())
		, dim_sizes(dims_array.size() + 1)
	{
		dim_sizes.front() = 1;
		for (std::size_t i = 1; i < dim_sizes.size(); ++i)
		{
			dim_sizes[i] = dim_sizes[i - 1] * dims_array[i - 1];
		}
		
		vertices.resize(dim_sizes.back());
		for (vertex_id i = 0; i < vertices.size(); ++i)
		{
			vertices[i] = vertex(i, *this);
		}
	} 
	
	// Returns true iff vid is an element on the outer shell of the hypercube.
	[[nodiscard]] bool is_on_outer_shell(vertex_id vid) const
	{
		return vertices[vid].neighbors.size() != dims_array.size() * 2;
	}
	
	// Vertices
	std::vector<vertex> vertices;

	// Dimension array passed into constructor
	const std::vector<vertex_id> dims_array;
	
	private:
	// Accumulated size of each dimension
	std::vector<vertex_id> dim_sizes;
};
