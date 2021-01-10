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
std::strong_ordering slice_base<T,dims...>::compareSymmetries
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
	for (auto v : s.forms[0][0])
	{
		stream << (slice_defs::empty(v) ? '_' : 'X');
	}
	return stream;
}

template<std::unsigned_integral T, T ... dims>
void pruned_slice<T,dims...>::emplace_symmetry(
	const typename slice_base<T,dims...>::compNumArray& sym)
{
	// Place it in either:
	// 1: A new vector in forms, if it is physically distinct from the rest.
	// 2: In an existing vector in forms, if it physically the same
	//    as another, but the array itself is different.
	// 3: Nowhere, if it is exactly the same as another array.
	
	if (auto it = std::find_if(forms.begin(),forms.end(),
		[&sym](const auto& physForm)
		{
			return slice_base<T,dims...>::compareSymmetries(physForm[0],sym) == 0;
		});
		it == forms.end())
	{
		// Physical form not found (case 1)
		forms.emplace_back().emplace_back(sym);
	}
	else if (std::find(it->begin(),it->end(),sym) == it->end())
	{
		// Physical form found, but this exact symmetry does not exist (case 2)
		it->emplace_back(sym);
	}
	// Otherwise, this exact symmetry has already been found (case 3)
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
			auto& s = slices.emplace_back(path,nv);
			
			// Minor todo: this might be faster if this is pruned as we go along
			if (std::find(s.forms[0][0].begin(), s.forms[0][0].end(),
				slice_defs::COMPLETELY_EMPTY) != s.forms[0][0].end())
			{
				slices.pop_back();
				return;
			}
			
			// Out param for calls to permute
			typename slice_base<T,dims...>::compNumArray result;
			
			// Produce each symmetry. Should one of them be lexicographically smaller
			// than this one, then remove it. Start at index 1, since 0 is the identity.
			for (unsigned i = 1; i < permutationSet<T,dims...>::perms.size(); ++i)
			{
				slice_base<T,dims...>::permute(i,s.forms[0][0],result);
				
				// If this new symmetry is lexicographically smaller
				// than the original, then we can prune this one.
				if (slice_base<T,dims...>::compareSymmetries(s.forms[0][0],result) > 0)
				{
					slices.pop_back();
					return;
				}
				
				// Otherwise, place it in the slice
				s.emplace_symmetry(result);
			}
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
			// Go through each symmetry
			for (const auto& symSet : slices[i].forms)
			{
				if (slice_base<T,dims...>::succeeds(symSet[0], slices[i].numComps,
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
							slice_base<T,dims...>::succeeds(symSet[j],
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
