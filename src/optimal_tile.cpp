#include "slice.hpp"
#include "fraction.hpp"

// A macro named DIM_SIZES will be compiled in.

struct path_info
{
	// Number of induced vertices in this path, the vertex
	// that precedes this one in this path.
	unsigned num_induced, second_to_last;
};

// Index [len][end] gives info about the 'len'
// long path with 'end' as the last vertex.
using path_info_matrix = std::vector<std::vector<path_info>>;

template<std::unsigned_integral T, T ... dims>
struct tile
{
	std::vector<unsigned> slices;
	fraction density;
	
	tile() {}
	
	tile(const path_info_matrix& paths_info, unsigned len,
		unsigned end, fraction dens) : slices(len), density(dens)
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
std::ostream& operator<<(std::ostream& stream, const tile<T,dims...>& t)
{
	stream << t.density << '\n';
	for (unsigned vertex : t.slices)
	{
		stream << slice_graph<true,T,dims...>::lookup(vertex)
			<< slice_defs::er_store
				[slice_graph<true,T,dims...>::graph[vertex].erID]
			<< "\n\n";
	}
	return stream;
}

/*------------------------------------
Prints the maximum size and density of
every rectangle sized 1xn through nxn.
Also, prints the maximum density tile.
------------------------------------*/

template<std::unsigned_integral T, T ... dims>
void findMaxTilingWithStart(unsigned start, tile<T,dims...>& bestTile)
{
	using slice = slice_graph<true,T,dims...>;
	
	// Information matrix, size is nearly square. The inner vector needs to be
	// the same size as the slice graph. The outer vector needs to be one larger,
	// so that when enumerating tiles, all tiles with all slices would technically
	// be accounted for.
	
	// TODO: The outer vector likely can be smaller, experimentally no maximum tiles
	// have been longer than 10. Perhaps grow slowly, occasionally checking for
	// duplicate slices.
	path_info_matrix paths_info(
		slice::graph.size() + 1, std::vector<path_info>(slice::graph.size(),{0,0})
	);
	
	constexpr unsigned num_verts = (dims * ...);
	
	// Only initialize a single cell, since here we have a specific starting vertex.
	paths_info[1][start].num_induced = slice::lookup(start).numVerts;
	
	// For each length
	for (unsigned len = 2; len < paths_info.size(); ++len)
	{
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
						
						if (density > bestTile.density)
						{
							bestTile = tile<T,dims...>(paths_info, len - 1,
								end, density);
							
							std::cout << "found: " << bestTile;
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
	tile<T,dims...> bestTile;
	
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
