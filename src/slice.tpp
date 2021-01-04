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
			return sym1_induced <=> sym2_induced;
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

template<std::unsigned_integral T, T ... dims>
pruned_slice<T,dims...>::pruned_slice(bool v) : slice_base<T,dims...>(v),
	forms({{{v ? static_cast<slice_defs::compNumType>(0)
	           : slice_defs::COMPLETELY_EMPTY}}}) {}

template<std::unsigned_integral T, T d1, T ... rest>
void slice_base<T,d1,rest...>::constructForm(const std::vector<unsigned>& path,
	compNumArray& out)
{
	constexpr static T subNV = slice_defs::sub_slice<T,d1,rest...>::pset::numVertices;
	
	// Count the total number of components in all slices. We don't need
	// to concatenate the exact ERs, just the number of components is needed.
	unsigned totalComponents = 0;
	for (unsigned vID : path)
	{
		totalComponents += slice_defs::sub_graph<T,d1,rest...>::lookup(vID).numComps;
	}
	
	equivRelation combination(totalComponents);
	
	// Total offset before both columns, and offset between them.
	unsigned base_offset = 0, offset;
	
	// Iterate over each pair of adjacent columns
	for (unsigned i = 0; i < d1 - 1; ++i)
	{
		const auto& ss1 = slice_defs::sub_graph<T,d1,rest...>::lookup(path[i]);
		const auto& ss2 = slice_defs::sub_graph<T,d1,rest...>::lookup(path[i + 1]);
		
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
		const auto& ss = slice_defs::sub_graph<T,d1,rest...>::lookup(vID);
		
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
	constexpr unsigned subDimSize = pset::numVertices/d1;
	for (unsigned i = 0; i < pset::numVertices; ++i)
	{
		if (out[i] == slice_defs::COMPLETELY_EMPTY)
		{
			if ((i >= subDimSize &&
			       !slice_defs::empty(out[i - subDimSize])) ||
			    (i < pset::numVertices - subDimSize &&
			       !slice_defs::empty(out[i + subDimSize]))
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
	for (const auto& symmetryClass : s.forms)
	{
		for (auto v : symmetryClass.front())
		{
			stream << (slice_defs::empty(v) ? '_' : 'X');
		}
		stream << '\n';
	}
	return stream;
}

template<std::unsigned_integral T, T d1, T ... rest>
std::ostream& operator<<(std::ostream& stream, const pruned_slice<T,d1,rest...>& s)
{
	for (const auto& symmetryClass : s.forms)
	{
		for (auto v : symmetryClass.front())
		{
			stream << (slice_defs::empty(v) ? '_' : 'X');
		}
		stream << '\n';
	}
	return stream;
}

template<std::unsigned_integral T, T d1, T ... rest>
pruned_slice<T,d1,rest...>::pruned_slice
	(const std::vector<unsigned>& path, unsigned nv)
		: slice_base<T,d1,rest...>(nv,0)
{
	forms.emplace_back().emplace_back();
	slice_base<T,d1,rest...>::constructForm(path,forms[0][0]);
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
	slice_defs::sub_graph<T,d1,rest...>::enumerate();
	const auto& subslices = slice_defs::sub_graph<T,d1,rest...>::slices;
	
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
	// If the path is the size of the primary dimension, add the slice.
	if (path.size() == d1)
	{
		if constexpr (prune)
		{
			auto& s = slices.emplace_back(path,nv);
			
			// Minor todo: this might be faster if this is pruned as we go along
			if (std::find(s.forms[0][0].begin(), s.forms[0][0].end(),
				slice_defs::COMPLETELY_EMPTY) != s.forms[0][0].end())
			{
				slices.pop_back();
				return;
			}
			
			// Out param for calls to permute
			typename slice_base<T,d1,rest...>::compNumArray result;
			
			// Produce each symmetry. Should one of them be lexicographically smaller
			// than this one, then remove it. Start at index 1, since 0 is the identity.
			for (unsigned i = 1; i < permutationSet<T,d1,rest...>::perms.size(); ++i)
			{
				slice_base<T,d1,rest...>::permute(i,s.forms[0][0],result);
				
				// If this new symmetry is lexicographically smaller
				// than the original, then we can prune this one.
				if (slice_base<T,d1,rest...>::compareSymmetries(s.forms[0][0],result) > 0)
				{
					slices.pop_back();
					return;
				}
			}
			
			// Otherwise, place it in either:
			// 1: A new vector in forms, if it is physically distinct from the rest.
			// 2: In an existing vector in forms, if it physically the same
			//    as another, but the array itself is different.
			// 3: Nowhere, if it is exactly the same as another array.
			for (auto& physForm : s.forms)
			{
				// If these are same physically, check further inwards
				if (slice_base<T,d1,rest...>::compareSymmetries(physForm[0],result) == 0)
				{
					// This rechecks the first index, but this is necessary because
					// compareSymmetries does a comparison of physical forms, here
					// we need to check exact values.
					for (const auto& config : physForm)
					{
						// If an exact match already exists here, stop. (case 3)
						// A goto is needed here to break out of two loops
						if (result == config) goto endloop;
					}
					
					// Otherwise, put cn at the back of this array and stop. (case 2)
					physForm.emplace_back(result);
					goto endloop;
				}
			}
			
			// If control reaches here, then no place was found for it, so put it in
			// a new top-level vector. (case 1)
			s.forms.emplace_back().emplace_back(result);
			
			endloop: ;
		}
		else
		{
			slices.emplace_back(path,nv);
		}
	}
	else
	{
		for (const auto& adj :
			slice_defs::sub_graph<T,d1,rest...>::graph[path.back()].adjList)
		{
			// Need to find and store the number
			// of vertices the adjacent slice has.
			const unsigned deltaNV =
				slice_defs::sub_graph<T,d1,rest...>::lookup(adj).numVerts;
			
			path.push_back(adj);
			nv += deltaNV;
			
			enumerateRecursive(path,nv);
			
			path.pop_back();
			nv -= deltaNV;
		}
	}
}

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
void slice_graph<prune,T,d1,rest...>::fillVertex(unsigned vID)
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
			slices.size() * permutationSet<T,d1,rest...>::perms.size());
		
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < slices.size(); ++i)
		{
			// Go through each symmetry
			for (const auto& symSet : slices[i].forms)
			{
				if (slice_base<T,d1,rest...>::succeeds(symSet[0], slices[i].numComps,
					lookup(vID).forms[0][0], graph[vID].erID, result))
				{
					// This is in this block so that multiple configurations of a slice
					// can be added to the adjacency list.
					
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
						
						// Iterate through all other versions of this
						// physical symmetry, map any generated configs
						// to the new vertex.
						for (unsigned j = 1; j < symSet.size(); ++j)
						{
							// No need to check the result, since the
							// same physical configuration will also work.
							slice_base<T,d1,rest...>::succeeds(symSet[j],
								slices[i].numComps, lookup(vID).forms[0][0],
								graph[vID].erID, result);
							
							slices[i].er_map[result] = adjacent;
						}
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
			if (slice_base<T,d1,rest...>::succeeds(slices[i].form,
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
