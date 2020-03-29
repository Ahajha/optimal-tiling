#ifndef DEFS_HPP
#define DEFS_HPP

#include <stack>

// This file contains basic type definitions along with headers for some functions
// shared between different programs, and 'global' variables.

template <std::size_t N>
class indexedList;

struct Subtree;

namespace defs
{
	typedef int vertexID;
	
	constexpr vertexID EMPTY = -1;
	constexpr unsigned numVertices = SIZEX*SIZEY*SIZEZ;
	
	enum action_type { add, rem, stop };
	struct action { action_type type; vertexID v; };
}

#endif
