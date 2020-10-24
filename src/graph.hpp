#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <array>
#include "semiarray.hpp"
#include "defs.hpp"

/*
In this case, a Graph is a cubic lattice, with size given
by SIZEX, SIZEY, and SIZEZ.
*/

class Graph
{
	// Will eventually be removed in favor of a template
	static constexpr std::array<defs::vertexID, 3> dim_array = { SIZEX,SIZEY,SIZEZ };
	
	public:
	
	struct graphVertex
	{
		// Neighbors are the vertices it actually
		// touches, directions are indexed according to
		// the direction enum below, EMPTY means there
		// is no vertex in a given direction.
		
		semiarray <defs::vertexID, dim_array.size() * 2> neighbors;
		std::array<defs::vertexID, dim_array.size() * 2> directions;
		
		graphVertex() {}
		graphVertex(defs::vertexID);
	};
	
	private:
	
	static std::array<graphVertex, SIZEX*SIZEY*SIZEZ> makeVertices();
	
	public:
	
	enum direction
	{
		// These are not arbitrary, they are in order of ascending ID.
		// Up and down     : z axis
		// North and south : y axis
		// East and west   : x axis
		UP    = 5,
		DOWN  = 0,
		NORTH = 4,
		SOUTH = 1,
		EAST  = 3,
		WEST  = 2
	};
	
	// Index of a given vertex is its ID
	const static inline std::array<graphVertex, defs::numVertices> vertices = makeVertices();
	
	[[nodiscard]] static bool onOuterShell(defs::vertexID);
	
	[[nodiscard]] static constexpr defs::vertexID sizeof_dim(unsigned d);
	[[nodiscard]] static constexpr defs::vertexID get_coord (unsigned d, defs::vertexID c);
	
	[[nodiscard]] static constexpr defs::vertexID forward   (unsigned d, defs::vertexID c);
	[[nodiscard]] static constexpr defs::vertexID backward  (unsigned d, defs::vertexID c);
};

#endif
