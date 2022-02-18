#include "graph.hpp"

constexpr Graph::vertexID Graph::sizeof_dim(unsigned d)
{
	return std::accumulate(dim_array.begin(), dim_array.begin() + d,
		1, std::multiplies<vertexID>());
}

constexpr Graph::vertexID Graph::get_coord(unsigned d, vertexID c)
{
	return (c / sizeof_dim(d)) % dim_array[d];
}

constexpr Graph::vertexID Graph::forward  (unsigned d, vertexID c)
{
	return (get_coord(d,c) == dim_array[d] - 1)
		? EMPTY : c + sizeof_dim(d);
}

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

bool Graph::onOuterShell(vertexID c)
{
	return vertices[c].neighbors.size() != dim_array.size() * 2;
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
