#ifndef SUBTREE_HPP
#define SUBTREE_HPP

#include <array>
#include <vector>
#include <iostream>

#include "defs.hpp"
#include "graph.hpp"

// Represents an induced subtree
struct Subtree
{
	struct subTreeVertex
	{
		bool induced;
		unsigned effectiveDegree;
		
		subTreeVertex() : induced(false), effectiveDegree(0) {}
	};
	
	// Each index is either enabled or disabled, and includes its
	// effective degree (which is cnt)
	
	unsigned numInduced;
	
	defs::vertexID root;
	
	std::array<subTreeVertex, defs::numVertices> vertices;
	
	Subtree(defs::vertexID);
	
	unsigned cnt(defs::vertexID i) const { return vertices[i].effectiveDegree; }
	bool     has(defs::vertexID i) const { return vertices[i].induced;         }
	
	// Does nothing if the graph would be invalidated
	bool add(defs::vertexID);
	
	void rem(defs::vertexID);
	
	bool exists(defs::vertexID i) const { return i != defs::EMPTY && vertices[i].induced; }
	
	void print() const;
	
	void writeToFile(std::string filename) const;
	
	// Defined only for dimensions 2 and 3. A vertex is valid
	// if it has at most one axis with both neighbors.
	bool validate(defs::vertexID i) const;
	
	// Returns true iff there is at least one block whose
	// faces cannot be accessed externally.
	bool hasEnclosedSpace() const;
	
	// Returns true iff adding i would preserve the neighbor condition.
	bool safeToAdd(defs::vertexID);
};

#endif
