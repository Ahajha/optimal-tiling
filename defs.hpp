#ifndef DEFS_HPP
#define DEFS_HPP

#include "CTPL/ctpl_stl.h"

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
	
	extern const unsigned NUM_THREADS;
	
	// Thread pool
	extern ctpl::thread_pool pool;
	
	// Maximum size graph seen so far
	extern unsigned largestTree, largestWithEnclosed;
	
	// File to write the best graph seen so far to
	extern std::string outfile;
	
	// The start time of the program
	extern clock_t start_time;
	
	// Grid of indexedLists, used to store the border elements as they are removed,
	// then swapped back to restore. A call to branch can find the list it should
	// use by going to lists[id][S.numInduced].
	extern std::vector<
		std::array<indexedList<defs::numVertices>, defs::numVertices>
	> lists;
	
	// Used to store the number of leaves seen thus far
	extern std::vector<unsigned long long> numLeaves;
	extern bool lastWasNew;
	
	// Returns the number of thread-seconds since the start of the program.
	float threadSeconds();
	
	// Updates the border of S after adding x.
	void update(Subtree& S, indexedList<numVertices>& border,
		vertexID x, std::stack<action>& previous_actions);
	
	// Restores the border of S after removing x.
	void restore(indexedList<numVertices>& border,
		std::stack<action>& previous_actions);
}

#endif
