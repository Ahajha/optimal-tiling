#include "slice.hpp"
#include "slice_algo_base.hpp"

// A macro named DIM_SIZES will be compiled in.

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
	
	std::cout << slice_path<T,rest...>(paths_info,d1,bestEndVertex)
		<< paths_info[d1][bestEndVertex].num_induced << std::endl;
}

int main()
{
	auto start_time = clock();
	
	enumerate<unsigned,DIM_SIZES>(start_time);
	
	findMaxHypercube<unsigned,DIM_SIZES>();
	
	std::cout << "Finished in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
}
