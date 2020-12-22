#include "slice.hpp"

template<auto dims>
	requires unsigned_range<decltype(dims)>
void slice_base<dims>::permute(unsigned permID, const compNumArray& src,
	compNumArray& result)
{
	const auto& perm = pset::perms[permID];
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		result[i] = src[perm[i]];
	}
}

template<auto dims>
	requires unsigned_range<decltype(dims)>
std::strong_ordering slice_base<dims>::compareSymmetries
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

template<auto dims>
	requires unsigned_range<decltype(dims)>
bool slice_base<dims>::succeeds(const compNumArray& afterCN,
	unsigned afterNumComp, const compNumArray& beforeCN,
	unsigned beforeERID, unsigned& result)
{
	const equivRelation& beforeConfig = slice_defs::er_store[beforeERID];
	
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
	result = slice_defs::er_store(combination.shave(beforeConfig.size()));
	
	return true;
}

template<auto d>
	requires unsigned_range<decltype(d)>
std::ostream& operator<<(std::ostream& stream, const unpruned_slice<d>& s)
{
	for (auto v : s.form)
	{
		stream << (slice_defs::empty(v) ? 'X' : '_');
	}
	return stream;
}

template<auto dims>
	requires unsigned_range<decltype(dims)>
unpruned_slice<dims>::unpruned_slice
	(const std::vector<unsigned>& path, unsigned nv)
		: slice_base<dims>::numVerts(nv)
{
	using subSlice = slice_graph<slice_base<dims>::pset::subArray, false>;
	constexpr unsigned dimSize = *dims.rbegin();
	
	// Count the total number of components in all slices. We don't need
	// to concatenate the exact ERs, just the number of components is needed.
	unsigned totalComponents = 0;
	for (unsigned vID : path)
	{
		totalComponents += subSlice::lookup(vID).numComps;
	}
	
	equivRelation combination(totalComponents);
	
	// Total offset before both columns, and offset between them.
	unsigned base_offset = 0, offset;
	
	// Iterate over each pair of adjacent columns
	for (unsigned i = 0; i < path.size() - 1; ++i)
	{	
		const auto& col1 = subSlice::lookup(path[i]);
		const auto& col2 = subSlice::lookup(path[i + 1]);
		
		offset = col1.numComponents;
		
		// Iterate over the two adjacent columns, if both vertices exist,
		// then merge their respective components. This is already known
		// to not have cycles, so no need to check.
		for (unsigned j = 0; j < dimSize; ++j)
		{
			if (col1[j] && col2[j])
			{
				combination.merge(base_offset + col1.componentNums[j],
				         offset + base_offset + col2.componentNums[j]);
			}
		}
		
		base_offset += offset;
	}
	
	slice_base<dims>::numComps = combination.numComponents();
	
	const auto& cgl = combination.canonicalGroupLabeling();
	
	// Fill in componentNums based on the component
	// numbers and the canonical group labeling.
	unsigned pos = 0;
	base_offset = 0;
	for (unsigned vID : path)
	{
		const auto& col = subSlice::lookup(vID);
	
		for (unsigned j = 0; j < dimSize; ++j)
		{
			// Re-use col[j] if it is empty, in case it is completely empty.
			form[pos++] = slice_defs::empty(col[j])
				? col[j]
				: cgl[col.componentNums[j] + base_offset];
		}
		
		base_offset += col.numComponents;
	}
	
	// Set any completely empty vertices to empty if they have a vertex
	// on either side of them. (By induction, they don't have any to the
	// side of them in any other dimensions)
	for (unsigned i = 0; i < form.size(); ++i)
	{
		if (form[i] == slice_defs::COMPLETELY_EMPTY)
		{
			if (!(
				(i < dimSize ||
					slice_defs::empty(form[i - dimSize])) &&
			    (i + dimSize >= form.size() ||
			    	slice_defs::empty(form[i + dimSize]))
			     )
			   )
			{
				form[i] = slice_defs::EMPTY;
			}
		}
	}
}

template<auto d>
	requires unsigned_range<decltype(d)>
std::ostream& operator<<(std::ostream& stream, const pruned_slice<d>& s)
{
	return stream;
}

template<auto dims>
	requires unsigned_range<decltype(dims)>
pruned_slice<dims>::pruned_slice
	(const std::vector<unsigned>& path, unsigned nv)
{
	
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice_graph<dims,prune>::enumerate()
{
	/*
	// This likely will not happen, but just to
	// be safe, avoid multiple calls to this.
	if (!slices.empty()) return;
	
	if constexpr (dims.empty())
	{
		// There are two different physical forms,
		// each with a single configuration.
		slices.resize(2);
		slices[0] = {{{{slice_defs::COMPLETELY_EMPTY}}}, 0, 0};
		slices[1] = {{{{0}}}, 1, 1};
		
		addVertex(0, slice_defs::er_store(equivRelation(0)));
		addVertex(1, slice_defs::er_store(equivRelation(1)));
		
		// Both slices can succeed one another.
		graph[0].adjList = {0, 1};
		graph[1].adjList = {0, 1};
	}
	else
	{
		// For now, assume no pruning
		
		const auto& subslices = unpruned_slice<pset::subArray>::slices;
		for (unsigned i = 0; i < subslices.size(); ++i)
		{
			std::vector<unsigned> path { i };
			unsigned nv = subslices[i].numVerts;
			
			enumerateRecursive(path,nv);
		}
	}
	*/
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice_graph<dims,prune>::enumerateRecursive
	(std::vector<unsigned>& path,unsigned nv)
{
	if constexpr (prune)
	{
	
	}
	else
	{
	
	}
	
	/*
	// Buffer for output to succeeds
	compNumArray cna;

	// The last dimension is the "primary" one.
	if (path.size() == *dims.rbegin())
	{
		// Produce the slice
		auto& sliceGroup = slices.emplace_back(path,nv);
		
		// Produce each symmetry. Should one of them be lexicographically smaller
		// than this one, then remove it.
	}
	*/
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice_graph<dims, prune>::fillVertex(unsigned vID)
{
	/*
	// Out-parameter for 'succeeds' function calls
	unsigned result;
	
	// Go through each of the physical columns (as the afters),
	// and see if it can succeed this configuration.
	for (unsigned i = 0; i < slices.size(); i++)
	{
		// Forms[0][0] gives the 'default' form, with no components
		// connected.
		if (succeeds(slices[i].forms[0][0], slices[i].numComponents,
			lookup(vID).componentNums, graph[vID].erID, result))
		{
			auto search = slices[i].er_map.find(result);
			
			if (search != slices[i].er_map.end())
			{
				// Found
				graph[vID].adjList.push_back(search->second);
			}
			else
			{
				// Not found
				graph[vID].adjList.push_back(graph.size());
				
				addVertex(i,result);
			}
		}
	}
	*/
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
auto slice_graph<dims, prune>::lookup(unsigned vID) -> slice_t&
{
	return slices[graph[vID].sliceNum];
}

template<auto dims, bool prune>
	requires unsigned_range<decltype(dims)>
void slice_graph<dims,prune>::addVertex(unsigned sliceID, unsigned erID)
{
	slices[sliceID].er_map[erID] = graph.size();
	graph.emplace_back(sliceID,erID);
}
