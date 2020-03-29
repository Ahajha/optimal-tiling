#ifndef DEFS_HPP
#define DEFS_HPP

#include <stack>

// This file contains basic type definitions along with headers for some functions
// shared between different programs, and 'global' variables.

template <std::size_t N>
class indexedList;

struct Subtree;
struct Graph;

namespace defs
{
	typedef int vertexID;
	
	constexpr vertexID EMPTY = -1;
	constexpr unsigned numVertices = SIZEX*SIZEY*SIZEZ;
	
	enum action_type { add, rem, stop };
	struct action { action_type type; vertexID v; };
	
	extern const Graph G;
	
	// Updates the border of S after adding x.
	void update(Subtree& S, indexedList<numVertices>& border,
		vertexID x, std::stack<action>& previous_actions);
	
	// Restores the border of S after removing x.
	void restore(indexedList<numVertices>& border,
		std::stack<action>& previous_actions);
}

#endif
