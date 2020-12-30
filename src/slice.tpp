#include "slice.hpp"

template<std::unsigned_integral T, T d1, T ... rest>
void slice_base<T,d1,rest...>::permute(unsigned permID, const compNumArray& src,
	compNumArray& result)
{
	const auto& perm = pset::perms[permID];
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		result[i] = src[perm[i]];
	}
}

template<std::unsigned_integral T, T d1, T ... rest>
std::strong_ordering slice_base<T,d1,rest...>::compareSymmetries
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
		bool sym1_induced = slice_defs::empty(sym1[i]);
		bool sym2_induced = slice_defs::empty(sym2[i]);
		if (sym1_induced != sym2_induced)
			return sym1[i] <=> sym2[i];
	}
	return std::strong_ordering::equal;
}

template<std::unsigned_integral T, T d1, T ... rest>
bool slice_base<T,d1,rest...>::succeeds(const compNumArray& afterCN,
	unsigned afterNumComp, const compNumArray& beforeCN,
	unsigned beforeERID, unsigned& result)
{
	const equivRelation& beforeConfig = slice_defs::er_store[beforeERID];
	
	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	
	equivRelation combination = equivRelation(afterNumComp).append(beforeConfig);
	
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		// If both vertices exist
		if (!slice_defs::empty(afterCN[i]) && !slice_defs::empty(beforeCN[i]))
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

template<std::unsigned_integral T, T ... dims>
std::ostream& operator<<(std::ostream& stream, const unpruned_slice<T,dims...>& s)
{
	for (auto v : s.form)
	{
		stream << (slice_defs::empty(v) ? '_' : 'X');
	}
	return stream;
}

template<std::unsigned_integral T, T d1, T ... rest>
std::ostream& operator<<(std::ostream& stream, const unpruned_slice<T,d1,rest...>& s)
{
	for (auto v : s.form)
	{
		stream << (slice_defs::empty(v) ? '_' : 'X');
	}
	return stream;
}

template<std::unsigned_integral T, T ... dims>
slice_base<T,dims...>::slice_base(bool v) : numVerts(v), numComps(v) {}

template<std::unsigned_integral T, T d1, T ... rest>
slice_base<T,d1,rest...>::slice_base(unsigned nv, slice_defs::compNumType nc) :
	numVerts(nv), numComps(nc) {}

template<std::unsigned_integral T, T ... dims>
unpruned_slice<T,dims...>::unpruned_slice(bool v) : slice_base<T,dims...>(v),
	form({v ? static_cast<slice_defs::compNumType>(0)
	        : slice_defs::COMPLETELY_EMPTY}) {}

template<std::unsigned_integral T, T d1, T ... rest>
void slice_base<T,d1,rest...>::constructForm(const std::vector<unsigned>& path,
	compNumArray& out)
{
	using subGraph = slice_graph<false,T,rest...>;
	constexpr static T subNV = slice_base<T,rest...>::pset::numVertices;
	
	// Count the total number of components in all slices. We don't need
	// to concatenate the exact ERs, just the number of components is needed.
	unsigned totalComponents = 0;
	for (unsigned vID : path)
	{
		totalComponents += subGraph::lookup(vID).numComps;
	}
	
	equivRelation combination(totalComponents);
	
	// Total offset before both columns, and offset between them.
	unsigned base_offset = 0, offset;
	
	// Iterate over each pair of adjacent columns
	for (unsigned i = 0; i < d1 - 1; ++i)
	{
		const auto& ss1 = subGraph::lookup(path[i]);
		const auto& ss2 = subGraph::lookup(path[i + 1]);
		
		offset = ss1.numComps;
		
		// Iterate over the two adjacent columns, if both vertices exist,
		// then merge their respective components. This is already known
		// to not have cycles, so no need to check.
		for (unsigned j = 0; j < subNV; ++j)
		{
			if (!slice_defs::empty(ss1.form[j]) && !slice_defs::empty(ss2.form[j]))
			{
				combination.merge(base_offset + ss1.form[j],
				         offset + base_offset + ss2.form[j]);
			}
		}
		
		base_offset += offset;
	}
	
	numComps = combination.numComponents();
	
	const auto& cgl = combination.canonicalGroupLabeling();
	
	// Fill in componentNums based on the component
	// numbers and the canonical group labeling.
	unsigned pos = 0;
	base_offset = 0;
	for (unsigned vID : path)
	{
		const auto& ss = subGraph::lookup(vID);
		
		for (unsigned j = 0; j < subNV; ++j)
		{
			// Re-use ss.form[j] if it is empty, in case it is completely empty.
			out[pos++] = slice_defs::empty(ss.form[j])
				? ss.form[j]
				: cgl[ss.form[j] + base_offset];
		}
		
		base_offset += ss.numComps;
	}
	
	// Set any completely empty vertices to empty if they have a vertex
	// on either side of them. (By induction, they don't have any to the
	// side of them in any other dimensions)
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		if (out[i] == slice_defs::COMPLETELY_EMPTY)
		{
			if (!
			    ((i < d1                      || slice_defs::empty(out[i - d1])) &&
			     (i + d1 >= pset::numVertices || slice_defs::empty(out[i + d1])))
			   )
			{
				out[i] = slice_defs::EMPTY;
			}
		}
	}
}

template<std::unsigned_integral T, T d1, T ... rest>
unpruned_slice<T,d1,rest...>::unpruned_slice
	(const std::vector<unsigned>& path, unsigned nv)
		: slice_base<T,d1,rest...>(nv,0)
{
	slice_base<T,d1,rest...>::constructForm(path,form);
}

template<std::unsigned_integral T, T ... dims>
std::ostream& operator<<(std::ostream& stream, const pruned_slice<T,dims...>& s)
{
	return stream;
}

template<std::unsigned_integral T, T d1, T ... rest>
pruned_slice<T,d1,rest...>::pruned_slice
	(const std::vector<unsigned>& path, unsigned nv)
{
	
}

template<bool prune, std::unsigned_integral T, T ... dims>
void slice_graph<prune,T,dims...>::enumerate()
{
	if (!slices.empty()) return;
	
	// There are two different physical forms
	slices.emplace_back(false);
	slices.emplace_back(true);
	
	addVertex(0, slice_defs::er_store(equivRelation(0)));
	addVertex(1, slice_defs::er_store(equivRelation(1)));
	
	// Both slices can succeed one another.
	graph[0].adjList = {0, 1};
	graph[1].adjList = {0, 1};
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
void slice_graph<prune,T,d1,rest...>::enumerate()
{
	// This likely will not happen, but just to
	// be safe, avoid multiple calls to this.
	if (!slices.empty()) return;
	
	// Subslices are never pruned
	slice_graph<false,T,rest...>::enumerate();
	const auto& subslices = slice_graph<false,T,rest...>::slices;
	
	for (unsigned i = 0; i < subslices.size(); ++i)
	{
		std::vector<unsigned> path { i };
		unsigned nv = subslices[i].numVerts;
		
		enumerateRecursive(path,nv);
	}
	
	// Create base versions of all vertices
	for (unsigned i = 0; i < slices.size(); ++i)
	{
		addVertex(i,slice_defs::er_store(equivRelation(slices[i].numComps)));
	}
	
	// Fill in all adjacency lists
	for (unsigned i = 0; i < graph.size(); ++i)
	{
		fillVertex(i);
	}
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
void slice_graph<prune,T,d1,rest...>::enumerateRecursive
	(std::vector<unsigned>& path,unsigned& nv)
{
	if constexpr (prune)
	{
		// TODO
		/*
		// Produce the slice
		auto& sliceGroup = slices.emplace_back(path,nv);
		
		// Produce each symmetry. Should one of them be lexicographically smaller
		// than this one, then remove it.
		*/
	}
	else
	{
		// If the path is the size of the primary dimension, add the slice.
		if (path.size() == d1)
		{
			slices.emplace_back(path,nv);
		}
		else
		{
			for (const auto& adj :
				slice_graph<false,T,rest...>::graph[path.back()].adjList)
			{
				// Need to find and store the number
				// of vertices the adjacent slice has.
				const unsigned deltaNV =
					slice_graph<false,T,rest...>::lookup(adj).numVerts;
				
				path.push_back(adj);
				nv += deltaNV;
				
				enumerateRecursive(path,nv);
				
				path.pop_back();
				nv -= deltaNV;
			}
		}
	}
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
void slice_graph<prune,T,d1,rest...>::fillVertex(unsigned vID)
{
	if constexpr (prune)
	{
		/*
		// Buffer for output to succeeds
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
	else
	{
		// Buffer for output to succeeds
		unsigned result;
		
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < slices.size(); i++)
		{
			if (slice_base<T,d1,rest...>::succeeds(slices[i].form,
				slices[i].numComps, lookup(vID).form, graph[vID].erID, result))
			{
				// See if the configuration found exists already
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
	}
}

template<bool prune, std::unsigned_integral T, T ... dims>
auto slice_graph<prune,T,dims...>::lookup(unsigned vID) -> slice_t&
{
	return slices[graph[vID].sliceNum];
}

template<bool prune, std::unsigned_integral T, T ... dims>
void slice_graph<prune,T,dims...>::addVertex(unsigned sliceID, unsigned erID)
{
	slices[sliceID].er_map[erID] = graph.size();
	graph.emplace_back(sliceID,erID);
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
auto slice_graph<prune,T,d1,rest...>::lookup(unsigned vID) -> slice_t&
{
	return slices[graph[vID].sliceNum];
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
void slice_graph<prune,T,d1,rest...>::addVertex(unsigned sliceID, unsigned erID)
{
	slices[sliceID].er_map[erID] = graph.size();
	graph.emplace_back(sliceID,erID);
}
