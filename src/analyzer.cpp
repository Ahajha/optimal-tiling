#include <limits>
#include <vector>
#include <iostream>
#include <exception>

constexpr char INDUCED_VERTEX = 'X', EMPTY_VERTEX   = '_';

class CubicLattice
{
	enum vertexLabel
	{
		induced,
		inducedConnected,
		empty,
		emptyConnected,
		disabled
	};
	
	struct vertex
	{
		vertexLabel label;
		unsigned char degree;
		std::vector<unsigned> adjList;
		
		vertex() : label(empty), degree(0) {}
	};
	
	constexpr static unsigned EMPTY = std::numeric_limits<unsigned>::max();
	
	// The length of one side of the lattice, and its square and cube.
	unsigned size, s2, s3;
	
	// Dims for the length of each axis, and the size of a cross section
	// up to a given dimension.
	std::vector<unsigned> dims, dimSizes;
	std::vector<vertex> graph;
	
	unsigned _numInduced;
	
	// Extracts the d-dimension coordinate from an index.
	unsigned get_coord(unsigned d, unsigned index) const
	{
		return (index / dimSizes[d]) % dims[d];
	}
	
	// Starting at the current vertex (graph[index]), finds the first vertex with
	// a given label and stores that into index. Returns true if a vertex
	// with the given label was found, and false if not (and index = s3))
	bool getFirstWithLabel(unsigned& index, vertexLabel label) const
	{
		for (; index < s3; index++)
			if (graph[index].label == label)
				return true;
		return false;
	}
	
	// North and south are in the y direction, east and west in x, and
	// up and down in z.
	bool hasNorth(unsigned index) const { return get_coord(1,index) != size - 1; }
	bool hasSouth(unsigned index) const { return get_coord(1,index) != 0;        }
	bool hasEast (unsigned index) const { return get_coord(0,index) != size - 1; }
	bool hasWest (unsigned index) const { return get_coord(0,index) != 0;        }
	bool hasUp   (unsigned index) const { return get_coord(2,index) != size - 1; }
	bool hasDown (unsigned index) const { return get_coord(2,index) != 0;        }
	
	bool hasForward(unsigned d, unsigned index) const
	{
		return get_coord(d, index) != dims[d] - 1;
	}
	
	bool hasBackward(unsigned d, unsigned index) const
	{
		return get_coord(d, index) != 0;
	}
	
	// Returns true iff the label represents a selected block.
	static bool exists(vertexLabel label)
	{
		return label == inducedConnected || label == induced;
	}
	
	bool hasNorthNeighbor(unsigned index) const
		{ return hasNorth(index) && exists(graph[north(index)].label); }
	bool hasSouthNeighbor(unsigned index) const
		{ return hasSouth(index) && exists(graph[south(index)].label); }
	bool hasEastNeighbor (unsigned index) const
		{ return hasEast (index) && exists(graph[east (index)].label); }
	bool hasWestNeighbor (unsigned index) const
		{ return hasWest (index) && exists(graph[west (index)].label); }
	bool hasUpNeighbor   (unsigned index) const
		{ return hasUp   (index) && exists(graph[up   (index)].label); }
	bool hasDownNeighbor (unsigned index) const
		{ return hasDown (index) && exists(graph[down (index)].label); }
	
	// Each of these assumes its respective direction is valid.
	unsigned north(unsigned index) const { return index + size; }
	unsigned south(unsigned index) const { return index - size; }
	unsigned east (unsigned index) const { return index + 1;    }
	unsigned west (unsigned index) const { return index - 1;    }
	unsigned up   (unsigned index) const { return index + s2;   }
	unsigned down (unsigned index) const { return index - s2;   }
	
	unsigned forward(unsigned d, unsigned index) const
	{
		return (get_coord(d,index) == dims[d] - 1) ? EMPTY : index + dimSizes[d];
	}
	
	unsigned backward(unsigned d, unsigned index) const
	{
		return (get_coord(d,index) == 0) ? EMPTY : index - dimSizes[d];
	}
		
	// Uses a depth-first-search to mark all connected vertices
	// in the graph with a given label with a new label.
	void mark_connected(unsigned index, vertexLabel oldLabel, vertexLabel newLabel)
	{
		if (graph[index].label == oldLabel)
		{
			graph[index].label = newLabel;
			
			for (unsigned i : graph[index].adjList)
			{
				if (i != EMPTY)
				{
					mark_connected(i, oldLabel, newLabel);
				}
			}
		}
	}
	
	public:
	
	// Constructs a cubic lattice of side length s, and sets
	// all vertices to empty.
	CubicLattice(unsigned s) : size(s), s2(s*s), s3(s2*s),
		dims{ s, s, s }, dimSizes(3 + 1), _numInduced(0)
	{
		dimSizes[0] = 1;
		for (unsigned i = 0; i < dims.size(); ++i)
		{
			dimSizes[i + 1] = dimSizes[i] * dims[i];
		}
		
		graph.resize(dimSizes.back());
		
		for (unsigned i = 0; i < graph.size(); ++i)
		{
			auto& adjList = graph[i].adjList;
			
			adjList.resize(2 * dims.size());
			
			// The highest dimension has the largest and smallest neighbors.
			// The second highest dimension has the second largest
			// and second smallest neighbors, etc.
			// Neighbors on the same axis are on 'mirror' indexes.
			for (unsigned d = 0; d < dims.size(); ++d)
			{
				adjList[dims.size() - d - 1] = backward(d,i);
				adjList[dims.size() + d    ] = forward (d,i);
			}
		}
	}
	
	// Adds an induced vertex at a given index
	void add(unsigned index)
	{
		graph[index].label = induced;
		
		++_numInduced;
		
		for (unsigned i : graph[index].adjList)
		{
			if (i != EMPTY)
			{
				++graph[i].degree;
			}
		}
	}
	
	// Returns the number of vertices in the graph.
	unsigned numVertices() const
	{
		return dimSizes.back();
	}
	
	// Returns the number of induced vertices in the graph.
	unsigned numInduced() const
	{
		return _numInduced;
	}
	
	bool isConnected()
	{
		unsigned index = 0;
		
		// This will return false when the graph is empty, we say that
		// an empty graph is connected.
		if(!getFirstWithLabel(index,induced))
			return true;
		
		mark_connected(index,induced,inducedConnected);
		
		// There should no longer be any vertices with the 'induced' label.
		return !getFirstWithLabel(index,induced);
	}
	
	// Returns true iff all induced vertices have no more than three
	// neighbors in a given plane.
	bool validateNeighbors() const
	{
		for (unsigned i = 0; i < s3; i++)
		{
			if (exists(graph[i].label))
			{
				if (graph[i].degree < 4) continue;
				if (graph[i].degree > 4) return false;
				
				// Ensure it has a neighbor on each axis
				if (!hasNorthNeighbor(i) && !hasSouthNeighbor(i) ||
				    !hasEastNeighbor (i) && !hasWestNeighbor (i) ||
				    !hasUpNeighbor   (i) && !hasDownNeighbor (i))
					return false;
			}
		}
		return true;
	}
	
	// Returns true iff there are block(s) whose faces cannot be
	// accessed externally.
	bool hasEnclosedSpace()
	{
		// This could be made slightly faster by only looking
		// at each 'face' of the graph, but that would be
		// a much more complicated implementation. Either way,
		// the algorithm is still O(nv).
		for (unsigned i = 0; i < s3; i++)
		{
			// Initialize a search in all vertices that are missing neighbors, i.e.
			// neighbors on the outer shell of the graph.
			if (std::find(graph[i].adjList.begin(), graph[i].adjList.end(), EMPTY)
				== graph[i].adjList.end())
			{
				mark_connected(i, empty, emptyConnected);
			}
		}
		
		// getFirstWithLabel needs a reference to a variable
		unsigned index = 0;
		return getFirstWithLabel(index, empty);
	}
	
	// Assumes that isConnected() has been called and returned true,
	// and hasEnclosedSpace() has been called.
	unsigned numFaces() const
	{
		unsigned sum = 0;
		
		for (unsigned i = 0; i < s3; i++)
		{
			switch (graph[i].label)
			{
				case inducedConnected: sum += (6 - graph[i].degree); break;
				case empty:            sum -= graph[i].degree; break;
			}
		}
		return sum;
	}
};

void readSizes(std::istream& stream, std::vector<unsigned>& result)
{
	for (int c; (c = stream.peek()) != '\n' && stream;)
	{
		if ('0' <= c && c <= '9')
		{
			unsigned& value = result.emplace_back();
			stream >> value;
		}
		else stream.ignore();
	}
}

int main()
{
	std::vector<unsigned> sizes;
	readSizes(std::cin, sizes);
	
	CubicLattice graph(sizes[0]);
	
	char symbol;
	for (unsigned i = 0; i < graph.numVertices(); ++i)
	{
		if (!(std::cin >> symbol))
		{
			std::cerr << "error reading input, are there enough vertices?\n";
			return 1;
		}
		
		if (symbol == INDUCED_VERTEX)
		{
			graph.add(i);
		}
	}
	
	std::cout << std::boolalpha;
	
	unsigned numVert = graph.numInduced();
	std::cout << numVert << " induced vertices" << std::endl;
	
	std::cout << "Graph is connected: "
		<< graph.isConnected() << std::endl;
	
	std::cout << "Graph satisfies neighbor condition: "
		<< graph.validateNeighbors() << std::endl;
	
	std::cout << "Graph has enclosed space: "
		<< graph.hasEnclosedSpace() << std::endl;
	
	unsigned numFaces = graph.numFaces();
	std::cout << "Number of faces: " << numFaces << std::endl;
	
	std::cout << "Graph is a tree: "
		<< (numFaces == 4 * numVert + 2) << std::endl;
}
