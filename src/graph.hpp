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
	static constexpr std::array<unsigned, 3> dim_array = { SIZEX,SIZEY,SIZEZ };
	
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
	
	[[nodiscard]] static bool onOuterShell(vertexID);
	
	[[nodiscard]] static constexpr vertexID sizeof_dim(unsigned d);
	[[nodiscard]] static constexpr vertexID get_coord (unsigned d, vertexID c);
	
	[[nodiscard]] static constexpr vertexID forward   (unsigned d, vertexID c);
	[[nodiscard]] static constexpr vertexID backward  (unsigned d, vertexID c);
};

#endif
