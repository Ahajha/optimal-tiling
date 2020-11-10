#include "slice.hpp"

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice<dims,prune>::enumerate()
{
	// This likely will not happen, but just to
	// be safe, avoid multiple calls to this.
	if (!slices.empty()) return;
	
	if constexpr (dims.empty())
	{
		// There are two different physical forms,
		// each with a single configuration.
		slices.resize(2);
		slices[0] = {{{{COMPLETELY_EMPTY}}}, 0, 0};
		slices[1] = {{{{0}}}, 1, 1};
		
		addVertex(0, er_store(equivRelation(0)));
		addVertex(1, er_store(equivRelation(1)));
		
		// Both slices can succeed one another.
		graph[0].adjList = {0, 1};
		graph[1].adjList = {0, 1};
	}
	else
	{
		// TODO
	}
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice<dims,prune>::addVertex(unsigned sliceID, unsigned erID)
{
	slices[sliceID].er_map[erID] = graph.size();
	graph.emplace_back(sliceID,erID);
}
