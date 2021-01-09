#include "slice.hpp"

// A macro named DIM_SIZES will be compiled in.

struct path_info
{
	// Number of induced vertices in this path, and the vertex
	// that precedes this one in this path.
	unsigned num_induced, second_to_last;
};

// Index [len][end] gives info about the 'len'
// long path with 'end' as the last vertex.
using path_info_matrix = std::vector<std::vector<path_info>>;

template<std::unsigned_integral T, T ... dims>
struct hypercube
{
	std::vector<unsigned> slices;
	unsigned num_induced;
	
	hypercube(const path_info_matrix& paths_info, unsigned len,
		unsigned end) : slices(len), num_induced(paths_info[len][end].num_induced)
	{
		unsigned currentVertex = end;
		for (unsigned length = len; length > 0; length--)
		{
			slices[length - 1] = currentVertex;
			
			currentVertex = paths_info[length][currentVertex].second_to_last;
		}
	}
};

template<std::unsigned_integral T, T ... dims>
std::ostream& operator<<(std::ostream& stream, const hypercube<T,dims...>& hc)
{
	for (unsigned vertex : hc.slices)
	{
		stream << slice_graph<true,T,dims...>::lookup(vertex)
			<< slice_defs::er_store
				[slice_graph<true,T,dims...>::graph[vertex].erID]
			<< "\n\n";
	}
	return stream << hc.num_induced;
}

template<std::unsigned_integral T, T, T ... rest>
void enumerate(auto start_time)
{
	slice_graph<true,T,rest...>::enumerate();
	
	std::cout << slice_graph<true,T,rest...>::slices.size()
		<< " slices generated and adjacency lists "
		<< "filled in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
}

/*--------------------------------------------------------
Prints the tile with the maximum number of acyclic induced
vertices of a hypercube with a given set of dimensions.
--------------------------------------------------------*/

template<std::unsigned_integral T, T d1, T ... rest>
void findMaxHypercube()
{
	using slice = slice_graph<true,T,rest...>;
	
	// Information matrix, size is d1 + 1 (so the last index is d1)
	// by the size of the slice graph.
	path_info_matrix paths_info(
		d1 + 1, std::vector<path_info>(slice::graph.size())
	);

	// For each starting slice (with default configs)
	for (unsigned start = 0; start < slice::graph.size(); ++start)
	{
		// Set the size of the 1-wide rectangle with just 1 column.
		paths_info[1][start].num_induced = slice::lookup(start).numVerts;
	}
	
	// For each length
	for (unsigned len = 2; len <= d1; ++len)
	{
		// Try to expand each cell that has a valid path
		for (unsigned end = 0; end < slice::graph.size(); ++end)
		{
			const unsigned oldNV = paths_info[len - 1][end].num_induced;
			
			// Expand in every possible way
			for (unsigned adj : slice::graph[end].adjList)
			{
				// Reference, for brevity
				path_info& info = paths_info[len][adj];
				
				// The number of induced vertices after the slice is added
				const unsigned newNV = oldNV + slice::lookup(adj).numVerts;
				
				if (info.num_induced < newNV)
				{
					info = {newNV, end};
				}
			}
		}
	}
	
	unsigned bestEndVertex = 0, maxNumVerts = 0;
	for (unsigned end = 0; end < slice::graph.size(); ++end)
	{
		if (paths_info[d1][end].num_induced > maxNumVerts)
		{
			bestEndVertex = end;
			maxNumVerts = paths_info[d1][end].num_induced;
		}
	}
	
	std::cout << hypercube<T,rest...>(paths_info,d1,bestEndVertex) << std::endl;
}

int main()
{
	auto start_time = clock();
	
	enumerate<unsigned,DIM_SIZES>(start_time);
	
	findMaxHypercube<unsigned,DIM_SIZES>();
	
	std::cout << "Finished in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
}
