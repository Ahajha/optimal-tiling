#ifndef DEFS_HPP
#define DEFS_HPP

#include "../CTPL/ctpl_stl.h"
#include "graph.hpp"
#include "subTree.hpp"
#include "indexedList.hpp"

#include <stack>
#include <ctime>
#include <mutex>

// This file contains basic type definitions along with headers for some functions
// shared between different programs, and 'global' variables.

namespace defs
{
	enum action_type { add, rem, stop };
	struct action { action_type type; Graph::vertexID v; };
	
	inline const int NUM_THREADS = std::thread::hardware_concurrency();
	
	// Thread pool
	inline ctpl::thread_pool pool(NUM_THREADS);
	
	// Maximum size graph seen so far
	inline unsigned largestTree = 0, largestWithEnclosed = 0;
	
	// File to write the best graph seen so far to
	inline std::string outfile {};
	
	// The start time of the program
	inline clock_t start_time {};
	
	// Grid of indexedLists, used to store the border elements as they are removed,
	// then swapped back to restore. A call to branch can find the list it should
	// use by going to lists[id][S.numInduced].
	inline std::vector<
		std::array<indexedList<Graph::vertexID, Graph::numVertices>, Graph::numVertices>
	> lists(NUM_THREADS);
	
	// Used to store the number of leaves seen thus far
	inline std::vector<uintmax_t> numLeaves(NUM_THREADS, 0);
	inline bool lastWasNew = false;
	
	// Used for thread safety on any IO actions.
	inline std::mutex IOmutex {};
	
	// Returns the number of thread-seconds since the start of the program.
	float threadSeconds();
	
	// Updates the border of S after adding x.
	void update(Subtree& S, indexedList<Graph::vertexID, Graph::numVertices>& border,
		Graph::vertexID x, std::stack<action>& previous_actions);
	
	// Restores the border of S after removing x.
	void restore(indexedList<Graph::vertexID, Graph::numVertices>& border,
		std::stack<action>& previous_actions);
	
	// After confirming S has a greater number of blocks than seen before,
	// prints to clog If S does not have enclosed space, updates
	// largestTree and writes the result to outfile, does neither if S
	// does have enclosed space.
	void checkCandidate(Subtree S);
}

#endif
