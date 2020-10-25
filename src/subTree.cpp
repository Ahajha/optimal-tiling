#include "graph.hpp"
#include "subTree.hpp"
#include <fstream>
#include <queue>

// These are used to print to the file
#define BLOCK_PRESENT 'X'
#define BLOCK_MISSING '_'

bool Subtree::add(defs::vertexID i)
{
	vertices[i].induced = true;
	
	// This should have one neighbor, we need to validate the neighbor
	for (const defs::vertexID x : Graph::vertices[i].neighbors)
	{
		if (has(x))
		{
			++vertices[x].effectiveDegree;
			
			if (validate(x)) break;
			else
			{
				// Undo changes made and report that this is invalid
				--vertices[x].effectiveDegree;
				vertices[i].induced = false;
				return false;
			}
		}
	}
	
	++numInduced;

	for (const defs::vertexID x : Graph::vertices[i].neighbors)
	{
		// Ignore the induced vertex, its degree has already been increased.
		if (!has(x))
			++vertices[x].effectiveDegree;
	}
	return true;
}

void Subtree::rem(defs::vertexID i)
{
	vertices[i].induced = false;
	
	--numInduced;
	
	for (const defs::vertexID x : Graph::vertices[i].neighbors)
	{
		--vertices[x].effectiveDegree;
	}
}

void Subtree::print() const
{
	std::cout << "Subgraph: ";
	for (defs::vertexID x = 0; x < defs::numVertices; x++)
	{
		if (has(x)) std::cout << x << ' ';
	}
	std::cout << std::endl;
}

void Subtree::writeToFile(std::string filename) const
{
	std::ofstream file(filename);

	file << SIZEX << ' ' << SIZEY << ' ' << SIZEZ << std::endl << std::endl;
	
	defs::vertexID x = 0;
	for (unsigned i = 0; i < SIZEZ; i++)
	{
		for (unsigned j = 0; j < SIZEY; j++)
		{
			for (unsigned k = 0; k < SIZEX; k++)
			{
				file << (has(x++) ? BLOCK_PRESENT : BLOCK_MISSING);
			}
			file << std::endl;
		}
		file << std::endl;
	}
	file << numInduced << std::endl;
}

Subtree::Subtree(defs::vertexID r) : numInduced(0), root(r), vertices()
{
	add(r);
}

bool Subtree::validate(defs::vertexID i) const
	// TODO: Requires dimension 2 or 3, use
	// if constexpr for different versions
{
	if (cnt(i) != 4) return cnt(i) < 4;
	
	auto& dirs = Graph::vertices[i].directions;
	
	// Ensure all axis have at least one neighbor
	for (unsigned d = 0; d < 3; ++d)
	{
		// 5 - d gets the opposite direction
		if (!exists(dirs[d]) && !exists(dirs[5 - d]))
			return false;
	}
	return true;	
}

bool Subtree::hasEnclosedSpace() const
{
	// enum to mark each vertex
	enum label { induced, empty, empty_connected };
	
	// Each vertex is labeled one of the above
	std::array<label, defs::numVertices> vertex_labels;
	
	// Initial label is either induced or empty
	for (defs::vertexID x = 0; x < defs::numVertices; x++)
	{
		vertex_labels[x] = has(x) ? induced : empty;
	}
	
	// Queue for breadth-first search
	std::queue<defs::vertexID> toBeVisited;
	
	// For each vertex touching the outer shell of the cube,
	// queue for searching
	for (defs::vertexID x = 0; x < defs::numVertices; x++)
	{
		if (Graph::onOuterShell(x))
		{
			toBeVisited.push(x);
		}
	}
	
	// Keep track of the number of connected empty vertices
	unsigned numConnected = 0;
	
	while (!toBeVisited.empty())
	{
		defs::vertexID x = toBeVisited.front();
		toBeVisited.pop();
		
		if (vertex_labels[x] == empty)
		{
			vertex_labels[x] = empty_connected;
			
			++numConnected;
			
			// Queue all of x's neighbors
			for (defs::vertexID y : Graph::vertices[x].neighbors)
			{
				toBeVisited.push(y);
			}
		}
	}
	
	// If the graph has enclosed space, then there will
	// be vertices not accounted for in this formula
	return numInduced + numConnected != defs::numVertices;
}

bool Subtree::safeToAdd(defs::vertexID i)
{
	vertices[i].induced = true;
	
	// This should have one neighbor, we need to validate the neighbor
	for (const defs::vertexID x : Graph::vertices[i].neighbors)
	{
		if (has(x))
		{
			++vertices[x].effectiveDegree;
			
			bool result = validate(x);
			
			// Undo changes made and report that this is invalid
			--vertices[x].effectiveDegree;
			vertices[i].induced = false;
			
			return result;
		}
	}
	
	return false;
}
