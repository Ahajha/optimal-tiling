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

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice<dims,prune>::permute(unsigned permID, const compNumArray& src,
	compNumArray& result)
{
	const auto& perm = pset::perms[permID];
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		result[i] = src[perm[i]];
	}
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
std::strong_ordering slice<dims,prune>::compareSymmetries
	(const compNumArray& sym1, const compNumArray& sym2)
{
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		// We only compare physical forms, as two physical
		// forms with different equivalence configurations are
		// functionally identical. Also, note that each physical form
		// is produced exactly once, meaning that if something
		// is equivalent physically to the base configuration,
		// we can still safely ignore it, as there is no other slice
		// it would be produced by.
		bool sym1_induced = sym1[i] >= 0;
		bool sym2_induced = sym2[i] >= 0;
		if (sym1_induced != sym2_induced)
			return sym1[i] <=> sym2[i];
	}
	return std::strong_ordering::equal;
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
bool slice<dims,prune>::succeeds(const compNumArray& afterCN,
	unsigned afterNumComp, const compNumArray& beforeCN,
	unsigned beforeERID, unsigned& result)
{
	const equivRelation& beforeConfig = er_store[beforeERID];
	
	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	equivRelation combination = equivRelation(afterNumComp).append(beforeERID);
	
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		// If both vertices exist
		if (afterCN[i] >= 0 && beforeCN[i] >= 0)
		{
			// Check if their associated equivalence classes
			// are already equivalent. If so, this would
			// cause a cycle, so return false.
			if (combination.equivalent(afterCN[i], afterNumComp + beforeCN[i]))
				return false;
			
			// Otherwise, merge them.
			combination.merge(afterCN[i], afterNumComp + beforeCN[i]);
		}
	}
	
	// Result is acyclic, so produce the ER that should be used
	// by shaving off the last 'before.numComponents' items.
	result = er_store(combination.shave(beforeConfig.size()));
	
	return true;
}
