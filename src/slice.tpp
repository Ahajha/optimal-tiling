#include "slice.hpp"

template<std::unsigned_integral T, T ... dims>
void slice_base<T,dims...>::permute(unsigned permID, const compNumArray& src,
	compNumArray& result)
{
	const auto& perm = pset::perms[permID];
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		result[i] = src[perm[i]];
	}
}

template<std::unsigned_integral T, T ... dims>
std::strong_ordering slice_base<T,dims...>::compNumArray::operator<=>
	(const compNumArray& other) const
{
	for (unsigned i = 0; i < slice_base<T,dims...>::pset::numVertices; ++i)
	{
		bool induced1 = slice_defs::empty((*this)[i]);
		bool induced2 = slice_defs::empty(other[i]);
		if (induced1 != induced2)
			return induced1 <=> induced2;
	}
	return std::strong_ordering::equal;
}

template<std::unsigned_integral T, T ... dims>
bool slice_base<T,dims...>::succeeds(const compNumArray& afterCN,
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
std::ostream& operator<<(std::ostream& stream, const pruned_slice<T,dims...>& s)
{
	for (auto v : s.forms[0])
	{
		stream << (slice_defs::empty(v) ? '_' : 'X');
	}
	return stream;
}

template<std::unsigned_integral T, T ... dims>
void slice_base<T,dims...>::constructForm(const std::vector<unsigned>& path,
	compNumArray& out)
{
	constexpr static T subNV = slice_alias<T,dims...>::sub_slice::pset::numVertices;
	
	// Count the total number of components in all slices. We don't need
	// to concatenate the exact ERs, just the number of components is needed.
	unsigned totalComponents = 0;
	for (unsigned vID : path)
	{
		totalComponents += slice_alias<T,dims...>::sub_graph::lookup(vID).numComps;
	}
	
	equivRelation combination(totalComponents);
	
	// Total offset before both columns, and offset between them.
	unsigned base_offset = 0, offset;
	
	// Iterate over each pair of adjacent columns
	for (unsigned i = 0; i < slice_alias<T,dims...>::primary_dim - 1; ++i)
	{
		const auto& ss1 = slice_alias<T,dims...>::sub_graph::lookup(path[i]);
		const auto& ss2 = slice_alias<T,dims...>::sub_graph::lookup(path[i + 1]);
		
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
		const auto& ss = slice_alias<T,dims...>::sub_graph::lookup(vID);
		
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
	constexpr unsigned subDimSize = pset::numVertices/slice_alias<T,dims...>::primary_dim;
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		if (out[i] == slice_defs::COMPLETELY_EMPTY &&
			((i >= subDimSize && !slice_defs::empty(out[i - subDimSize])) ||
			 (i < pset::numVertices - subDimSize && !slice_defs::empty(out[i + subDimSize]))
			))
		{
			out[i] = slice_defs::EMPTY;
		}
	}
}

// Returns true
template<std::unsigned_integral T, T ... dims>
bool pruned_slice<T,dims...>::fillOrPrune()
{
	if constexpr (sizeof...(dims) == 1)
	{
		/*-------------------------------------------------
		For 1 dimension, can implement both pruning methods
		fairly easily.
		
		Can use a DFA for this:

		State 0: 'X' (less than 2 spaces prior)
		X -> 0, _ -> 1

		State 1: '_' (exactly one space), has an X before
		  it, but no more than one space behind that)
		  (start state)
		X -> 0, _ -> 2

		State 2: '__' (exactly two spaces, X before,
		  no more than one space behind that)
		  (accepts if ending in this state)
		X -> 3, _ -> 5

		State 3: '__X'
		X -> 0, _ -> 4

		State 4: '__X_' (accepts if ending in this state)
		X -> 0, _ -> 5

		State 5: Accept (either ___ or __X__ has been seen)
		-------------------------------------------------*/
		
		unsigned state = 1;
		
		// Generated from the transitions given above
		constexpr static unsigned transition[] {0,1,0,2,3,5,0,4,0,5};
		
		for (const auto i : forms[0])
		{
			state = transition[(2 * state) + slice_defs::empty(i)];
			
			// Short circuit the 'live' state
			if (state == 5) return true;
		}
		
		// 2 and 4 are the accept states
		if (state == 2 || state == 4) return true;
	}
	else
	{
		// Minor todo: this might be faster if this is pruned as we go along
		if (std::find(forms[0].begin(), forms[0].end(),
			slice_defs::COMPLETELY_EMPTY) != forms[0].end())
		{
			return true;
		}
	}
	
	typename slice_base<T,dims...>::compNumArray result;
	
	// Produce each symmetry. Should one of them be lexicographically smaller
	// than this one, then remove it. Start at index 1, since 0 is the identity.
	for (unsigned i = 1; i < permutationSet<T,dims...>::perms.size(); ++i)
	{
		slice_base<T,dims...>::permute(i,forms[0],result);
		
		// If this new symmetry is lexicographically smaller
		// than the original, then we can prune this one.
		if (forms[0] > result)
		{
			return true;
		}
		
		// If the physical form is not already in forms,
		// add it, otherwise ignore it.
		if (std::find(forms.begin(), forms.end(), result) == forms.end())
		{
			forms.emplace_back(result);
		}
	}
	
	return false;
}

template<bool prune, std::unsigned_integral T, T ... dims>
void slice_graph<prune,T,dims...>::enumerate()
{
	// This likely will not happen, but just to
	// be safe, avoid multiple calls to this.
	if (!slices.empty()) return;

	if constexpr (sizeof...(dims) == 0)
	{
		// There are two different physical forms
		slices.emplace_back(false);
		slices.emplace_back(true);
	}
	else
	{
		using sub_graph = slice_alias<T,dims...>::sub_graph;
		sub_graph::enumerate();
		
		for (unsigned i = 0; i < sub_graph::slices.size(); ++i)
		{
			std::vector<unsigned> path { i };
			unsigned nv = sub_graph::slices[i].numVerts;
			
			enumerateRecursive(path,nv);
		}
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

template<bool prune, std::unsigned_integral T, T ... dims>
void slice_graph<prune,T,dims...>::enumerateRecursive
	(std::vector<unsigned>& path,unsigned& nv)
{
	// If the path is the size of the primary dimension, add the slice.
	if (path.size() == slice_alias<T,dims...>::primary_dim)
	{
		if constexpr (prune)
		{
			if (slices.emplace_back(path,nv).fillOrPrune())
				slices.pop_back();
		}
		else
		{
			slices.emplace_back(path,nv);
		}
	}
	else
	{
		for (const auto& adj :
			slice_alias<T,dims...>::sub_graph::graph[path.back()].adjList)
		{
			// Need to find and store the number
			// of vertices the adjacent slice has.
			const unsigned deltaNV =
				slice_alias<T,dims...>::sub_graph::lookup(adj).numVerts;
			
			path.push_back(adj);
			nv += deltaNV;
			
			enumerateRecursive(path,nv);
			
			path.pop_back();
			nv -= deltaNV;
		}
	}
}

template<bool prune, std::unsigned_integral T, T ... dims>
void slice_graph<prune,T,dims...>::fillVertex(unsigned vID)
{
	// Out-parameter for 'succeeds' function calls
	unsigned result;
	
	if constexpr (prune)
	{
		// adjacentTo[x] means that vertex x can follow this vertex.
		// The size given is an upper bound on the number of vertices that can
		// exist after this adjacency list is filled (this is based on upper
		// bounds on the number of iterations of the inner loops). The vector
		// is default filled with false.
		std::vector<bool> adjacentTo(graph.size() +
			slices.size() * permutationSet<T,dims...>::perms.size());
		
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < slices.size(); ++i)
		{
			// If the canonical form of the 'after' vertex is always used,
			// and 'before' is rotated instead, then calls to 'succeeds' will
			// always produce the same ER, so long as the symmetries are
			// physically identical. Thus, we only need to look at one version
			// of each 'before' symmetry.
			
			// Go through each symmetry in the 'before'
			for (const auto& form : lookup(vID).forms)
			{
				if (slice_base<T,dims...>::succeeds(slices[i].forms[0],
					slices[i].numComps, form, graph[vID].erID, result))
				{
					// TODO: Exclude configs that are 'supersets'
					// of other configs (and prove this is valid).
					
					auto search = slices[i].er_map.find(result);
					
					unsigned adjacent;
					
					if (search != slices[i].er_map.end())
					{
						// Found
						adjacent = search->second;
					}
					else
					{
						// Not found
						adjacent = graph.size();
						
						addVertex(i,result);
					}
					
					if (!adjacentTo[adjacent])
					{
						adjacentTo[adjacent] = true;
						
						graph[vID].adjList.emplace_back(adjacent);
					}
				}
			}
		}
	}
	else
	{
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < slices.size(); ++i)
		{
			if (slice_base<T,dims...>::succeeds(slices[i].form,
				slices[i].numComps, lookup(vID).form, graph[vID].erID, result))
			{
				// See if the configuration found exists already
				auto search = slices[i].er_map.find(result);
				
				if (search != slices[i].er_map.end())
				{
					// Found
					graph[vID].adjList.emplace_back(search->second);
				}
				else
				{
					// Not found
					graph[vID].adjList.emplace_back(graph.size());
					
					addVertex(i,result);
				}
			}
		}
	}
}
