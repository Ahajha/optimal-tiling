#include "slice.hpp"
#include "fraction.hpp"
#include "slice_path.hpp"

// What length to start checking for duplicate tiles.
constexpr unsigned CHECK_START = 10;

// A macro named DIM_SIZES will be compiled in.

// Checks for duplicate tiles in each path of a given length, and resets
// those that have duplicates. Return true iff there are still paths to
// explore.
bool checkForDuplicateTiles(path_info_matrix& paths_info, unsigned len)
{
	unsigned numPaths = 0;
	for (unsigned i = 0; i < paths_info[0].size(); ++i)
	{
		// Ignore paths that already do not exist
		if (paths_info[len][i].num_induced == 0) continue;
		
		++numPaths;
		
		std::vector<bool> used(paths_info[0].size());
		
		unsigned currentVertex = i;
		used[currentVertex] = true;
		for (unsigned length = len; length > 0; --length)
		{
			currentVertex = paths_info[length][currentVertex].second_to_last;
			
			if (used[currentVertex])
			{
				paths_info[len][i].num_induced = 0;
				--numPaths;
				break;
			}
			
			used[currentVertex] = true;
		}
	}
	
	return numPaths > 0;
}

/*------------------------------------
Prints the maximum size and density of
every rectangle sized 1xn through nxn.
Also, prints the maximum density tile.
------------------------------------*/

template<std::unsigned_integral T, T ... dims>
void findMaxTilingWithStart(unsigned start, auto& bestTile)
{
	using slice = slice_graph<true,T,dims...>;
	
	// Information matrix, size is nearly square. The inner vector needs to be
	// the same size as the slice graph. The outer vector needs to be one larger,
	// so that when enumerating tiles, all tiles with all slices would technically
	// be accounted for.
	
	path_info_matrix paths_info(
		slice::graph.size() + 2, std::vector<path_info>(slice::graph.size(),{0,0})
	);
	
	constexpr unsigned num_verts = (dims * ...);
	
	// Only initialize a single cell, since here we have a specific starting vertex.
	paths_info[1][start].num_induced = slice::lookup(start).numVerts;
	
	// For each length
	for (unsigned len = 2; len < paths_info.size(); ++len)
	{
		// This is expensive to check, so we will only check at each multiple
		// of CHECK_START.
		if (len % CHECK_START == 0 && !checkForDuplicateTiles(paths_info,len - 1))
			return;
		
		// Try to expand each cell that has a valid path
		for (unsigned end = 0; end < slice::graph.size(); ++end)
		{
			const unsigned oldNV = paths_info[len - 1][end].num_induced;
			
			// Skip over paths that haven't started yet.
			if (oldNV == 0) continue;
			
			// Expand in every possible way
			for (unsigned adj : slice::graph[end].adjList)
			{
				// Reference, for brevity
				path_info& info = paths_info[len][adj];
				
				// Number of vertices that are added with the new slice
				const unsigned newNV = oldNV + slice::lookup(adj).numVerts;
				
				if (info.num_induced < newNV)
				{
					info = {newNV, end};
					
					// If a cycle has been found, check to see if it is the new best
					if (adj == start)
					{
						fraction density(oldNV, (len - 1) * num_verts);
						
						if (density > bestTile.second)
						{
							bestTile = {
								slice_path<T,dims...>(paths_info, len - 1, end),
								density
							};
							
							std::cout << "found: " << density << '\n'
								<< slice_path<T,dims...>(paths_info, len - 1, end);
						}
					}
				}
			}
		}
	}
}

template<std::unsigned_integral T, T ... dims>
void findMaxTiling()
{
	std::pair<slice_path<T,dims...>,fraction> bestTile;
	
	for (unsigned start = 0; start < slice_graph<true,T,dims...>::slices.size(); ++start)
	{
		findMaxTilingWithStart<T,dims...>(start,bestTile);
	}
}

int main()
{
	auto start_time = clock();
	
	slice_graph<true,unsigned,DIM_SIZES>::enumerate();
	
	std::cout << slice_graph<true,unsigned,DIM_SIZES>::slices.size()
		<< " slices generated and adjacency lists "
		<< "filled in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
	
	findMaxTiling<unsigned,DIM_SIZES>();
	
	std::cout << "Finished in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
}
