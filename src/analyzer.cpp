#include <vector>
#include <iostream>
#include <exception>

#define SELECT 'X'
#define EMPTY '_'

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
	};
	
	// The length of one side of the lattice, and its square and cube.
	unsigned size, s2, s3;
	
	// Dims for the length of each axis, and the size of a cross section
	// up to a given dimension.
	std::vector<unsigned> dims, dimSizes;
	std::vector<vertex> graph;
	
	unsigned numVertices;
	
	// Returns the index of coordinate (x,y,z) in the graph.
	// Assumes the coordinate is valid.
	unsigned safe_index(unsigned x, unsigned y, unsigned z) const
	{
		return x + size*y + s2*z;
	}
	
	// Returns the index of coordinate (x,y,z) in the graph.
	// Checks to see if the coordinate is valid.
	unsigned check_index(unsigned x, unsigned y, unsigned z) const
	{
		if (x >= size || y >= size || z >= size)
			throw std::out_of_range("invalid coordinate");
	
		return safe_index(x,y,z);
	}
	
	// Extracts the coordinates of a raw index.
	unsigned get_x(unsigned coord) const { return coord % size; }
	unsigned get_y(unsigned coord) const { return (coord / size) % size; }
	unsigned get_z(unsigned coord) const { return coord / s2; }
	
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
	bool hasNorth(unsigned index) const { return get_y(index) != size - 1; }
	bool hasSouth(unsigned index) const { return get_y(index) != 0;        }
	bool hasEast (unsigned index) const { return get_x(index) != size - 1; }
	bool hasWest (unsigned index) const { return get_x(index) != 0;        }
	bool hasUp   (unsigned index) const { return get_z(index) != size - 1; }
	bool hasDown (unsigned index) const { return get_z(index) != 0;        }
	
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
		
	// Uses a depth-first-search to mark all connected vertices
	// in the graph with a given label with a new label.
	void mark_connected(unsigned index, vertexLabel oldLabel, vertexLabel newLabel)
	{
		if (graph[index].label == oldLabel)
		{
			graph[index].label = newLabel;
		
			if (hasNorth(index)) mark_connected(north(index),oldLabel,newLabel);
			if (hasSouth(index)) mark_connected(south(index),oldLabel,newLabel);
			if (hasEast (index)) mark_connected(east (index),oldLabel,newLabel);
			if (hasWest (index)) mark_connected(west (index),oldLabel,newLabel);
			if (hasUp   (index)) mark_connected(up   (index),oldLabel,newLabel);
			if (hasDown (index)) mark_connected(down (index),oldLabel,newLabel);
		}
	}
	
	public:
	
	// Constructs a cubic lattice of side length s, and sets
	// all vertices to empty.
	CubicLattice(unsigned s) : size(s), s2(s*s), s3(s2*s),
		dims{ s, s, s }, dimSizes(3 + 1), graph(s3+1), numVertices(0)
	{
		dimSizes[0] = 1;
		for (unsigned i = 0; i < dims.size(); ++i)
		{
			dimSizes[i + 1] = dimSizes[i] * dims[i];
		}
		
		// The one extra vertex represents 'outside'
		for (unsigned i = 0; i < s3; i++)
		{
			graph[i].label = empty;
			graph[i].degree = 0;
		}
	}
	
	// Adds an induced vertex at coordinate (x,y,z).
	void add(unsigned x, unsigned y, unsigned z)
	{
		unsigned index = check_index(x,y,z);
		graph[index].label = induced;
		
		numVertices++;
		
		if (hasNorth(index)) graph[north(index)].degree++;
		if (hasSouth(index)) graph[south(index)].degree++;
		if (hasEast (index)) graph[east (index)].degree++;
		if (hasWest (index)) graph[west (index)].degree++;
		if (hasUp   (index)) graph[up   (index)].degree++;
		if (hasDown (index)) graph[down (index)].degree++;
	}
	
	// Returns the number of induced vertices in the graph.
	unsigned getNumVertices() const
	{
		return numVertices;
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
		// at each face of the large cube, but that would be
		// a much more complicated implementation. Either way,
		// the algorithm is still O(size^3).
		for (unsigned i = 0; i < s3; i++)
		{
			// Initialize a search in all outside blocks to
			// mark anything that has outside access.
			if (!hasNorth(i) || !hasSouth(i) ||
			    !hasEast (i) || !hasWest (i) ||
			    !hasUp   (i) || !hasDown (i))
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
	for (unsigned i = 0; i < sizes[0]; i++)
	{
		for (unsigned j = 0; j < sizes[0]; j++)
		{
			for (unsigned k = 0; k < sizes[0]; k++)
			{
				std::cin >> symbol;
				
				if (symbol == SELECT)
				{
					graph.add(i,j,k);
				}
			}
		}
	}
	
	std::cout << std::boolalpha;
	
	unsigned numVert = graph.getNumVertices();
	std::cout << numVert << " vertices" << std::endl;
	
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
