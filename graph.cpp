#include "graph.hpp"

int Graph::get_x(defs::vertexID coord) { return coord % SIZEX; }
int Graph::get_y(defs::vertexID coord) { return (coord / SIZEX) % SIZEY; }
int Graph::get_z(defs::vertexID coord) { return coord / (SIZEX*SIZEY); }

defs::vertexID Graph::_west (defs::vertexID i)
	{ return (get_x(i) == 0)         ? defs::EMPTY : i - 1;             }
	
defs::vertexID Graph::_east (defs::vertexID i)
	{ return (get_x(i) == SIZEX - 1) ? defs::EMPTY : i + 1;             }

defs::vertexID Graph::_south(defs::vertexID i)
	{ return (get_y(i) == 0)         ? defs::EMPTY : i - SIZEX;         }

defs::vertexID Graph::_north(defs::vertexID i)
	{ return (get_y(i) == SIZEY - 1) ? defs::EMPTY : i + SIZEX;         }

defs::vertexID Graph::_down (defs::vertexID i)
	{ return (get_z(i) == 0)         ? defs::EMPTY : i - (SIZEX*SIZEY); }

defs::vertexID Graph::_up   (defs::vertexID i)
	{ return (get_z(i) == SIZEZ - 1) ? defs::EMPTY : i + (SIZEX*SIZEY); }

Graph::graphVertex::graphVertex(defs::vertexID i)
{
	directions[WEST ] = _west (i);
	directions[EAST ] = _east (i);
	
	directions[SOUTH] = _south(i);
	directions[NORTH] = _north(i);
	
	directions[DOWN ] = _down (i);
	directions[UP   ] = _up   (i);
	
	// Neighbors need to be in ascending order of ID
	for (defs::vertexID n : directions)
		if (n != defs::EMPTY)
			neighbors.push_back(n);
}

bool Graph::onOuterShell(defs::vertexID i)
{
	return vertices[i].neighbors.size() != 6;
}

std::array<Graph::graphVertex, SIZEX*SIZEY*SIZEZ> Graph::makeVertices()
{
	std::array<graphVertex, SIZEX*SIZEY*SIZEZ> vertices_;
	
	for (defs::vertexID i = 0; i < vertices_.size(); i++)
	{
		vertices_[i] = graphVertex(i);
	}
	
	return vertices_;
}
