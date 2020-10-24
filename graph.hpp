#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <array>
#include <vector>
#include "defs.hpp"

/*
In this case, a Graph is a cubic lattice, with size given
by SIZEX, SIZEY, and SIZEZ.
*/

class Graph
{
	public:
	
	struct graphVertex
	{
		// Neighbors are the vertices it actually
		// touches, directions are indexed according to
		// the direction enum below, EMPTY means there
		// is no vertex in a given direction.
		
		std::vector<defs::vertexID> neighbors;
		std::array <defs::vertexID, 6> directions;
		
		graphVertex() {}
		graphVertex(defs::vertexID);
	};
	
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
	
	private:
	
	static std::array<graphVertex, SIZEX*SIZEY*SIZEZ> makeVertices();
	
	public:
	
	// Index of a given vertex is its ID
	static inline std::array<graphVertex, defs::numVertices> vertices
		= makeVertices();
	
	static bool onOuterShell(defs::vertexID);
	
	private:
	
	static int get_x(defs::vertexID);
	static int get_y(defs::vertexID);
	static int get_z(defs::vertexID);
	
	// Returns the index of the vertex in a given
	// direction, or EMPTY if such a vertex does
	// not exist.
	static defs::vertexID _west (defs::vertexID);
	static defs::vertexID _east (defs::vertexID);
	static defs::vertexID _north(defs::vertexID);
	static defs::vertexID _south(defs::vertexID);
	static defs::vertexID _down (defs::vertexID);
	static defs::vertexID _up   (defs::vertexID);
};

#endif
