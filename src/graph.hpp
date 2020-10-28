#ifndef GRAPH_HPP
#define GRAPH_HPP

#include "semiarray.hpp"
#include "minFastType.hpp"

#include <array>
#include <numeric>

/*
In this case, a Graph is a cubic lattice, with size given
by SIZEX, SIZEY, and SIZEZ.
*/

class Graph
{
	// Will eventually be removed in favor of a template
	static constexpr std::array<unsigned, 3> dim_array = { SIZE };
	
	public:
	
	constexpr static auto numVertices = minFastType<
		std::accumulate(dim_array.begin(),dim_array.end(),
			1,std::multiplies<uintmax_t>())
	>::value;
	
	using vertexID = std::remove_const<decltype(numVertices)>::type;
	
	constexpr static vertexID EMPTY = std::numeric_limits<vertexID>::max();
	
	struct graphVertex
	{
		// Neighbors are the vertices it actually
		// touches, directions are indexed according to
		// the direction enum below, EMPTY means there
		// is no vertex in a given direction.
		
		semiarray <vertexID, dim_array.size() * 2> neighbors;
		std::array<vertexID, dim_array.size() * 2> directions;
		
		graphVertex() {}
		graphVertex(vertexID);
	};
	
	private:
	
	static std::array<graphVertex, numVertices> makeVertices();
	
	public:
	
	// Index of a given vertex is its ID
	const static inline auto vertices = makeVertices();
	
	// Returns true iff c is an element on the outer shell of the hypercube.
	[[nodiscard]] static bool onOuterShell(vertexID c);
	
	// Returns the number of vertices that would be in the graph if
	// it were truncated to a given number of dimensions Note that
	// if d == dim_array.size(), this is just the number of vertices.
	[[nodiscard]] static constexpr vertexID sizeof_dim(unsigned d);
	
	// Gets a specific dimension of the coordinate of c. Valid values of
	// d are 0 <= d < dim_array.size()
	[[nodiscard]] static constexpr vertexID get_coord (unsigned d, vertexID c);
	
	// Returns the vertexID of the vertex "forward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] static constexpr vertexID forward   (unsigned d, vertexID c);
	
	// Returns the vertexID of the vertex "backward" in a
	// dimension d from c, if it exists, EMPTY if not.
	[[nodiscard]] static constexpr vertexID backward  (unsigned d, vertexID c);
};

#endif
