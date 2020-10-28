#ifndef SUBTREE_HPP
#define SUBTREE_HPP

#include "graph.hpp"

#include <array>
#include <vector>
#include <iostream>

// Represents an induced subtree
struct Subtree
{
	private:
	
	// To be removed later, used as a stopgap solution before templatization
	static constexpr auto dim_array = std::to_array<unsigned>({ SIZE });
	
	public:
	
	struct subTreeVertex
	{
		bool induced;
		unsigned effectiveDegree;
		
		subTreeVertex() : induced(false), effectiveDegree(0) {}
	};
	
	// Each index is either enabled or disabled, and includes its
	// effective degree (which is cnt)
	
	unsigned numInduced;
	
	Graph::vertexID root;
	
	std::array<subTreeVertex, Graph::numVertices> vertices;
	
	Subtree(Graph::vertexID);
	
	unsigned cnt(Graph::vertexID i) const { return vertices[i].effectiveDegree; }
	bool     has(Graph::vertexID i) const { return vertices[i].induced;         }
	
	// Does nothing if the graph would be invalidated
	bool add(Graph::vertexID);
	
	void rem(Graph::vertexID);
	
	bool exists(Graph::vertexID i) const
		{ return i != Graph::EMPTY && vertices[i].induced; }
	
	void print() const;
	
	void writeToFile(std::string filename) const;
	
	// Defined only for dimensions 2 and 3. A vertex is valid
	// if it has at most one axis with both neighbors.
	bool validate(Graph::vertexID i) const;
	
	// Returns true iff there is at least one block whose
	// faces cannot be accessed externally.
	bool hasEnclosedSpace() const;
	
	// Returns true iff adding i would preserve the neighbor condition.
	bool safeToAdd(Graph::vertexID);
};

#endif
