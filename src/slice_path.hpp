#ifndef SLICE_ALGO_BASE_HPP
#define SLICE_ALGO_BASE_HPP

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
struct slice_path
{
	std::vector<unsigned> slices;
	
	slice_path() {}
	
	slice_path(const path_info_matrix& paths_info, unsigned len, unsigned end) :
		slices(len)
	{
		unsigned currentVertex = end;
		for (unsigned length = len; length > 0; --length)
		{
			slices[length - 1] = currentVertex;
			
			currentVertex = paths_info[length][currentVertex].second_to_last;
		}
	}
};

template<std::unsigned_integral T, T ... dims>
std::ostream& operator<<(std::ostream& stream, const slice_path<T,dims...>& sp)
{
	for (unsigned vertex : sp.slices)
	{
		stream << slice_graph<true,T,dims...>::lookup(vertex) << '\n';
	}
	return stream;
}

#endif
