#include "graph.hpp"

// Returns the number of vertices that would be in the graph if
// it were truncated to a given number of dimensions Note that
// if d == dim_array.size(), this is just the number of vertices.
constexpr Graph::vertexID Graph::sizeof_dim(unsigned d)
{
	return std::accumulate(dim_array.begin(), dim_array.begin() + d,
		1, std::multiplies<vertexID>());
}

// Gets a specific dimension of the coordinate of c. Valid values of
// d are 0 <= d < dim_array.size()
constexpr Graph::vertexID Graph::get_coord(unsigned d, vertexID c)
{
	return (c / sizeof_dim(d)) % dim_array[d];
}

// Returns the vertexID of the vertex "forward" in a
// dimension d from c, if it exists, EMPTY if not.
constexpr Graph::vertexID Graph::forward  (unsigned d, vertexID c)
{
	return (get_coord(d,c) == dim_array[d] - 1)
		? EMPTY : c + sizeof_dim(d);
}

// Returns the vertexID of the vertex "backward" in a
// dimension d from c, if it exists, EMPTY if not.
constexpr Graph::vertexID Graph::backward (unsigned d, vertexID c)
{
	return (get_coord(d,c) == 0)
		? EMPTY : c - sizeof_dim(d);
}

Graph::graphVertex::graphVertex(vertexID c)
{
	// The highest dimension has the largest and smallest neighbors.
	// The second highest dimension has the second largest
	// and second smallest neighbors, etc.
	for (unsigned d = 0; d < dim_array.size(); ++d)
	{
		directions[dim_array.size() - d - 1] = backward(d,c);
		directions[dim_array.size() + d    ] = forward (d,c);
	}
	
	// Neighbors need to be in ascending order of ID
	for (vertexID n : directions)
	{
		if (n != EMPTY)
		{
			neighbors.push_back(n);
		}
	}
}

bool Graph::onOuterShell(vertexID i)
{
	return vertices[i].neighbors.size() != dim_array.size() * 2;
}

std::array<Graph::graphVertex, Graph::numVertices> Graph::makeVertices()
{
	std::array<graphVertex, numVertices> vertices_;
	
	for (vertexID i = 0; i < vertices_.size(); i++)
	{
		vertices_[i] = graphVertex(i);
	}
	
	return vertices_;
}
