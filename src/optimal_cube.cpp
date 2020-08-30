#include <cstdlib>
#include <iostream>
#include <vector>
#include <compare>
#include <list>
#include <ctime>
#include <array>
#include <unordered_map>
#include "equivRelation.hpp"
#include "fraction.hpp"

// The current state of this program is, frankly, awful, and I apologize
// profusely. Right now I just need this to work, I will work on making
// it prettier later.

// ==========================================================
// Global variables, struct definitions, and printing functions.

unsigned n;

bool trace = false;

er_storage er_store;

typedef char componentNumType;

// Named constants
constexpr static int EMPTY = -1;
constexpr static int COMPLETELY_EMPTY = -2;

// A vertex in a graph. Contains info about which slice it represents.
struct vertex
{
	std::vector<unsigned> adjList;
	
	unsigned sliceNum;
	unsigned erID;
	
	vertex(unsigned sn, unsigned e) :
		sliceNum(sn), erID(e) {}
};

// The physical configuration of the column.
// Stored as an integer, the least significant
// n bits represent the column, with a 1 being
// filled in, and a 0 being empty.
// For example, XX_XX_X would be 1101101

struct column
{
	// Assumes number of used bits is n.
	// Contains a component number, or EMPTY or COMPLETELY_EMPTY
	std::vector<componentNumType> componentNums;
	
	unsigned numComponents;
	unsigned numVertices;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	column(unsigned p) : componentNums(n), numComponents(0),
		numVertices(0)
	{
		bool lastWasSpace = true;
		
		for (unsigned i = 0; i < n; i++)
		{
			bool currentIsVertex = (p >> i) & 1;
			
			if (currentIsVertex)
			{
				numVertices++;
				
				if (lastWasSpace)
				{
					numComponents++;
				}
			}
			
			componentNums[i] = currentIsVertex ? (numComponents - 1) : EMPTY;
			
			lastWasSpace = !currentIsVertex;
		}
		
		// Go through again and mark any vertices that are completely empty
		for (unsigned i = 0; i < n; i++)
		{
			if (componentNums[i] < 0 &&
				(i == 0 || componentNums[i-1] < 0) &&
				(i == n-1 || componentNums[i+1] < 0))
			{
				componentNums[i] = COMPLETELY_EMPTY;
			}
		}
	}
	
	bool operator[](unsigned i) const { return componentNums[i] >= 0; }
	
	static std::vector<vertex> graph;
	static std::vector<column> columns;
	
	// Gets a column associated with a vertex ID.
	static column& lookup(unsigned vID) { return columns[graph[vID].sliceNum]; }
	
	static void addVertex(unsigned colID, unsigned erID)
	{
		columns[colID].er_map[erID] = graph.size();
		graph.emplace_back(colID,erID);
	}
};

std::vector<vertex> column::graph = {};
std::vector<column> column::columns = {};

struct pathWithoutSymmetries
{
	std::vector<unsigned> path;
	unsigned numVertices;
};

struct hyperCube
{
	pathWithoutSymmetries slices;
	fraction density;
};

std::vector<hyperCube> bestHyperCubes;
hyperCube bestTiling;

struct slice
{
	// Inner vector is a vector of component numbers (or EMPTY/COMPLETELY_EMPTY).
	// Middle vector is configs with the same physical form, but not identical.
	// Outer vector is different physical forms.
	std::vector<std::vector<std::vector<componentNumType>>> componentNums;
	
	unsigned numComponents;
	unsigned numVertices;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;

	slice(const pathWithoutSymmetries& p) : componentNums(1),
		numVertices(p.numVertices)
	{
		componentNums.front().emplace_back(p.path.size() * n);
		
		// Count the total number of components in all slices. We don't need
		// to concatenate the exact ERs, just the number of components is needed.
		unsigned totalComponents = 0;
		for (unsigned vID : p.path)
		{
			totalComponents += column::columns[column::graph[vID].sliceNum].numComponents;
		}
		
		equivRelation combination(totalComponents);
		
		// Total offset before both columns, and offset between them.
		unsigned base_offset = 0, offset;
		
		// Iterate over each pair of adjacent columns
		for (unsigned i = 0; i < p.path.size() - 1; i++)
		{	
			const auto& col1 = column::columns[column::graph[p.path[i]].sliceNum];
			const auto& col2 = column::columns[column::graph[p.path[i + 1]].sliceNum];
			
			offset = col1.numComponents;
			
			// Iterate over the two adjacent columns, if both vertices exist, then merge
			// their respective components. This is already known to not have cycles, so
			// no need to check.
			for (unsigned j = 0; j < n; j++)
			{
				if (col1[j] && col2[j])
				{
					combination.merge(base_offset + col1.componentNums[j],
					         offset + base_offset + col2.componentNums[j]);
				}
			}
			
			base_offset += offset;
		}
		
		numComponents = combination.numComponents();
		
		const auto& cgl = combination.canonicalGroupLabeling();
		
		// Fill in componentNums based on the component
		// numbers and the canonical group labeling.
		unsigned pos = 0;
		base_offset = 0;
		for (unsigned i = 0; i < p.path.size(); i++)
		{
			const auto& col = column::columns[column::graph[p.path[i]].sliceNum];
		
			for (unsigned j = 0; j < n; j++)
			{
				componentNums[0][0][pos++] = col[j]
					? cgl[col.componentNums[j] + base_offset]
					: -1;
			}
			
			base_offset += col.numComponents;
		}
	}
	
	static std::vector<vertex> graph;
	static std::vector<slice> slices;
	
	// Gets a slice associated with a vertex ID.
	static slice& lookup(unsigned vID) { return slices[graph[vID].sliceNum]; }
	
	static void addVertex(unsigned sliceID, unsigned erID)
	{
		slices[sliceID].er_map[erID] = graph.size();
		graph.emplace_back(sliceID,erID);
	}
};

std::vector<vertex> slice::graph = {};
std::vector<slice> slice::slices = {};

// =======================================================================
// Output functions

std::ostream& operator<<(std::ostream& stream, column p)
{
	for (unsigned i = 0; i < n; i++)
	{
		stream << (p[i] ? 'X' : '_');
	}
	
	if (trace)
	{
		stream << ",";
		for (unsigned i = 0; i < n; i++)
		{
			if (p[i])
			{
				stream << ' ' << (int)p.componentNums[i];
			}
			else
			{
				stream << "  ";
			}
		}
	}
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const pathWithoutSymmetries& p)
{
	for (unsigned c : p.path)
	{
		stream << column::lookup(c) << std::endl;
	}
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const slice& s)
{
	for (unsigned i = 0; i < n; i++)
	{
		for (unsigned j = 0; j < n; j++)
		{
			std::cout << ((s.componentNums[0][0][i * n + j] >= 0) ? 'X' : '_') << ' ';
		}
		
		std::cout << "| ";
		
		for (unsigned j = 0; j < n; j++)
		{
			if (s.componentNums[0][0][i * n + j] >= 0)
			{
				std::cout << (int)s.componentNums[0][0][i * n + j] << ' ';
			}
			else
			{
				std::cout << "  ";
			}
		}
		std::cout << std::endl;
	}
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::vector<vertex>& graph)
{
	for (unsigned vID = 0; vID < graph.size(); vID++)
	{
		std::cout << vID << ':';		
		for (unsigned adj : graph[vID].adjList)
		{
			std::cout << ' ' << adj;
		}
		std::cout << std::endl;
	}
	return stream;
}

// =======================================================================
// Various functions

std::strong_ordering compareSymmetries(std::vector<componentNumType> sym1,
	std::vector<componentNumType> sym2)
{
	// The symmetries are assumed to be the same size
	for (unsigned i = 0; i < sym1.size(); i++)
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

void parseArgs(int argn, char** args)
{
	if (argn < 2 || argn > 3)
	{
		std::cerr << "usage: " << args[0] << " <N> [-t]" << std::endl;
		exit(1);
	}
	
	if (argn == 3)
	{
		if (std::string("-t") == args[2])
		{
			trace = true;
		}
		else
		{
			std::cerr << "unknown option \"" << args[2] << "\"" << std::endl;
			exit(1);
		}
	}
	
	n = atoi(args[1]);
	
	if (n == 0)
	{
		std::cerr << "error, n should be a positive integer" << std::endl;
		exit(2);
	}
}

void produceColumns()
{
	unsigned numCols = 1 << n;
	
	column::columns.reserve(numCols);
	
	for (unsigned i = 0; i < numCols; i++)
	{
		column::columns.emplace_back(i);
		
		if (trace)
		{
			std::cout << i << ": " << column::columns[i] << std::endl;
		}
	}
	if (trace) std::cout << std::endl;
}

/*
If the component vector 'afterCN' (which has 'afterNumComponents' components)
can succeed the component vector 'beforeCN', preconfigured with ER number
'beforeConfigID', returns true and places into 'result' the number of the
equivalence relation that the successor would need to be configured with.
Otherwise, returns false.
*/

bool succeeds(const std::vector<componentNumType>& afterCN,
	unsigned afterNumComponents, const std::vector<componentNumType>& beforeCN,
	unsigned beforeConfigID, unsigned& result)
{
	const equivRelation& beforeConfig = er_store[beforeConfigID];
	
	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	equivRelation combination = equivRelation(afterNumComponents).append(beforeConfig);
	
	for (unsigned i = 0; i < afterCN.size(); i++)
	{
		// If both vertices exist
		if (afterCN[i] >= 0 && beforeCN[i] >= 0)
		{
			// Check if their associated equivalence classes
			// are already equivalent. If so, this would
			// cause a cycle, so return false.
			if (combination.equivalent(afterCN[i], afterNumComponents + beforeCN[i]))
				return false;
			
			// Otherwise, merge them.
			combination.merge(afterCN[i], afterNumComponents + beforeCN[i]);
		}
	}
	
	// Result is acyclic, so produce the ER that should be used
	// by shaving off the last 'before.numComponents' items.
	result = er_store(combination.shave(beforeConfig.size()));
	
	return true;
}

void fillInColumnAdjLists()
{
	// Start by inserting the first 2^n vertices
	for (unsigned i = 0; i < column::columns.size(); i++)
	{
		column::addVertex(i,er_store(equivRelation(column::columns[i].numComponents)));
	}
	
	// Out-parameter for 'succeeds' function calls
	unsigned result;
	
	// Fill in each adjacency list. column::graph.size() may change with
	// each iteration, but will eventually stop growing.
	for (unsigned vID = 0; vID < column::graph.size(); vID++)
	{
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < column::columns.size(); i++)
		{
			// TODO simplify this call
			if (succeeds(column::columns[i].componentNums, column::columns[i].numComponents,
				column::lookup(vID).componentNums, column::graph[vID].erID, result))
			{
				auto search = column::columns[i].er_map.find(result);
				
				if (search != column::columns[i].er_map.end())
				{
					// Found
					column::graph[vID].adjList.push_back(search->second);
				}
				else
				{
					// Not found
					column::graph[vID].adjList.push_back(column::graph.size());
					
					column::addVertex(i,result);
				}
			}
		}
	}
	
	if (trace) std::cout << column::graph << std::endl;
}

bool prune(pathWithoutSymmetries& p)
{
	// Prune any slice with a non-induced vertex with no induced neighbors.
	// Look for 'completely empty' vertices.
	
	if (p.path.size() == 2) // Check first column
	{
		const auto& prev1 = column::lookup(p.path[1]).componentNums;
		const auto& prev2 = column::lookup(p.path[0]).componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev2[i] == COMPLETELY_EMPTY && prev1[i] < 0)
			{
				return true;
			}
		}
	}
	else if (p.path.size() > 2) // Check any middle column
	{
		const auto& prev1 = column::lookup(p.path[p.path.size()-1]).componentNums;
		const auto& prev2 = column::lookup(p.path[p.path.size()-2]).componentNums;
		const auto& prev3 = column::lookup(p.path[p.path.size()-3]).componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev2[i] == COMPLETELY_EMPTY && prev1[i] < 0 && prev3[i] < 0)
			{
				return true;
			}
		}
	}
	
	if (p.path.size() == n) // Check last column
	{
		const auto& prev1 = column::lookup(p.path[n-1]).componentNums;
		const auto& prev2 = column::lookup(p.path[n-2]).componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev1[i] == COMPLETELY_EMPTY && prev2[i] < 0)
			{
				return true;
			}
		}
	}
	return false;
}

static constexpr unsigned numPermutations(unsigned n)
{
	// Answer is 2^n * n!
	
	unsigned result = 1;
	
	for (unsigned i = 0; i < n; i++)
	{
		result *= (n * (i + 1));
	}
	
	return result;
}

// n >= 1
template<unsigned N>
static constexpr auto permutations()
{
	// For now, just hardcode in the 8 values needed for 3 dimensions.
	// In the future, for higher dimensions, look into one of these algorithms:
	// https://en.wikipedia.org/wiki/Heap%27s_algorithm#cite_note-3
	// https://en.wikipedia.org/wiki/Steinhaus%E2%80%93Johnson%E2%80%93Trotter_algorithm
	
	std::array<std::array<char,N>,numPermutations(N)> result;
	
	char src[8][2] = { {1,2},{1,-2},{-1,-2},{-1,2},{2,1},{2,-1},{-2,-1},{-2,1} };
	
	for (unsigned i = 0; i < result.size(); i++)
	{
		result[i] = std::to_array(src[i]);
	}
	
	return result;
}

static constexpr auto perms = permutations<2>();

// pID is the index of the permutation to apply in perms.
std::vector<componentNumType> applyPermutation(unsigned pID,
	std::vector<componentNumType> componentNums)
{
	std::vector<componentNumType> result(componentNums.size());
	
	const auto& perm = perms[pID];
	
	std::vector<unsigned> old_coord(2);
	std::vector<unsigned> new_coord(2);
	
	for (old_coord[0] = 0; old_coord[0] < n; old_coord[0]++)
	{
		for (old_coord[1] = 0; old_coord[1] < n; old_coord[1]++)
		{
			// todo: [0] can be computed in the outer loop,
			// [1] at the inner, etc. for higher dimensions.
			for (unsigned i = 0; i < 2; i++)
			{
				new_coord[i] = (perm[i] > 0)
					? (old_coord[abs(perm[i]) - 1])
					: (n - 1 - old_coord[abs(perm[i]) - 1]);
			}
			
			result[new_coord[0] * n + new_coord[1]] =
				componentNums[old_coord[0] * n + old_coord[1]];
		}
	}
	
	return result;
}

// todo: here is where the choice of length (for rectangles) would be made
void produceSlicesRecursive(pathWithoutSymmetries& p)
{
	if (prune(p)) return;

	if (p.path.size() == n)
	{	
		// convert to a slice
		slice::slices.emplace_back(p);
		
		// Reference, for brevity
		auto& componentNums = slice::slices.back().componentNums;
		
		// Produce each symmetry. Should one of them be lexicographically smaller
		// than this one, then remove it. Start at index 1, since 0 is the identity.
		for (unsigned i = 1; i < perms.size(); i++)
		{
			auto cn = applyPermutation(i, componentNums[0][0]);
		
			// If this new symmetry is lexicographically smaller than the original,
			// then we can prune this one.
			if (compareSymmetries(cn, componentNums[0][0]) < 0)
			{
				slice::slices.pop_back();
				return;
			}
			
			// Otherwise, place it in either:
			// 1: A new vector in componentNums, if it is physically distinct from the rest.
			// 2: In an existing vector in componentNums, if it physically the same
			//    as another, but the vector itself is different.
			// 3: Nowhere, if it is exactly the same as another vector.
			for (auto& physicalForm : componentNums)
			{
				// If these are same physically, check further inwards
				if (compareSymmetries(cn, physicalForm.front()) == 0)
				{
					// This rechecks the first index, but this is necessary because
					// compareSymmetries does a comparison of physical forms, here
					// we need to check exact values.
					for (auto& config : physicalForm)
					{
						// If an exact match already exists here, stop. (case 3)
						// A goto is needed here to break out of two loops
						if (cn == config) goto endloop;
					}
					
					// Otherwise, put cn at the back of this array and stop. (case 2)
					physicalForm.push_back(cn);
					goto endloop;
				}
			}
			
			// If control reaches here, then no place was found for it, so put it in
			// a new top-level vector. (case 1)
			componentNums.emplace_back();
			componentNums.back().push_back(cn);
			
			endloop: ;
		}
		
		if (trace)
		{
			std::cout << p;
			
			for (unsigned i = 0; i < componentNums.size(); i++)
			{
				std::cout << i << ":" << std::endl;
				for (auto& config : componentNums[i])
				{
					for (auto j : config)
					{
						if (j < 0) std::cout << "_ ";
						else       std::cout << (int)j << ' ';
					}
					std::cout << std::endl;
				}
			}
			std::cout << std::endl;
		}
		
		return;
	}
	
	// Iterate using each available adjacent column.
	for (unsigned adj : column::graph[p.path.back()].adjList)
	{
		unsigned numVertices = column::lookup(adj).numVertices;
	
		p.path.push_back(adj);
		p.numVertices += numVertices;
		
		produceSlicesRecursive(p);
		
		p.path.pop_back();
		p.numVertices -= numVertices;
	}
}

void produceSlices()
{
	// We only need to look at slices that start with unconfigured columns,
	// of which there are 2^n of.
	unsigned numStartColumns = 1 << n;
	for (unsigned i = 0; i < numStartColumns; i++)
	{
		pathWithoutSymmetries p;
		
		p.path.push_back(i);
		p.numVertices = column::columns[i].numVertices;
		
		produceSlicesRecursive(p);
	}
}

void fillSliceVertex(unsigned vID)
{
	// Out-parameter for 'succeeds' function calls
	unsigned result;
	
	// adjacentTo[x] means that vertex x can follow this vertex.
	// The size given is an upper bound on the number of vertices that can
	// exist after this adjacency list is filled (this is based on upper
	// bounds on the number of iterations of the inner loops). The vector
	// is default filled with false.
	std::vector<bool> adjacentTo(slice::graph.size() + slice::slices.size() * perms.size());
	
	// Go through each of the physical columns (as the afters),
	// and see if it can succeed this configuration.
	for (unsigned i = 0; i < slice::slices.size(); i++)
	{
		// Go through each symmetry
		for (unsigned symNum = 0; symNum < slice::slices[i].componentNums.size(); symNum++)
		{
			// TODO simplify this call
			if (succeeds(slice::slices[i].componentNums[symNum].front(),
				slice::slices[i].numComponents,
				slice::lookup(vID).componentNums.front().front(),
				slice::graph[vID].erID, result))
			{
				// This is in this block so that multiple configurations of a slice
				// can be added to the adjacency list.
				
				// TODO: Exclude configs that are 'supersets'
				// of other configs (and prove this is valid).
				
				auto search = slice::slices[i].er_map.find(result);
				
				unsigned adjacent;
				
				if (search != slice::slices[i].er_map.end())
				{
					// Found
					adjacent = search->second;
				}
				else
				{
					// Not found
					adjacent = slice::graph.size();
					
					slice::addVertex(i,result);
					
					// Iterate through all other versions of this
					// physical symmetry, map any generated configs
					// to the new vertex.
					for (unsigned j = 1; j < slice::slices[i].componentNums[symNum].size(); j++)
					{
						// No need to check the result, since the same physical configuration
						// will also work.
						succeeds(slice::slices[i].componentNums[symNum][j],
							slice::slices[i].numComponents,
							slice::lookup(vID).componentNums.front().front(),
							slice::graph[vID].erID, result);
						
						slice::slices[i].er_map[result] = adjacent;
					}
				}
				
				if (!adjacentTo[adjacent])
				{
					adjacentTo[adjacent] = true;
					
					slice::graph[vID].adjList.push_back(adjacent);
				}
			}
		}
	}
}

void fillInSliceAdjLists()
{
	// Start by inserting the first 2^n vertices
	for (unsigned i = 0; i < slice::slices.size(); i++)
	{
		slice::addVertex(i,er_store(equivRelation(slice::slices[i].numComponents)));
	}
	
	// Fill in each adjacency list. slice::graph.size() may change with
	// each iteration, but will eventually stop growing.
	for (unsigned vID = 0; vID < slice::graph.size(); vID++)
	{
		fillSliceVertex(vID);
	}
	
	if (trace) std::cout << slice::graph << std::endl;
}

struct path_info
{
	unsigned num_induced;
	
	// The vertex that precedes this one in the optimal path.
	unsigned second_to_last;
	
	unsigned start;
	
	path_info() : num_induced(0), second_to_last(0), start(0) {}
	
	path_info(unsigned n_i, unsigned s_t_l, unsigned s) :
		num_induced(n_i), second_to_last(s_t_l), start(s) {}
};

// All path info for one length and starting vertex.
// .info[s] gives the information about the path from
// the predetermined start column to s, of a predetermined length.
struct path_info_vector
{
	std::vector<path_info> info;
	
	path_info_vector() : info(slice::graph.size()) {}
};

// .lengths[len].info[e] gives
// the info block for the densest path from
// the given start column to e of length len.
struct path_info_matrix
{
	std::vector<path_info_vector> lengths;
	
	// This +2 is just a buffer, it might only need to be +1.
	// Experimentally, most results rarely need more than length 10
	// (in 2 dimensions), but I have not proven this to be a valid
	// pruning method.
	path_info_matrix() : lengths(slice::graph.size() + 2) {}
};

hyperCube extractHyperCube(const path_info_matrix& paths_info, unsigned len,
	unsigned end, fraction density)
{
	hyperCube h;
	
	h.slices.path.resize(len);
	
	unsigned currentVertex = end;
	for (unsigned length = len; length > 0; length--)
	{
		h.slices.path[length - 1] = currentVertex;
		
		currentVertex = paths_info.lengths[length].info[currentVertex].second_to_last;
	}
	
	h.density = density;
	
	return h;
}

/*------------------------------------
Prints the maximum size and density of
every rectangle sized 1xn through nxn.
Also, prints the maximum density tile.
------------------------------------*/

void enumerate()
{
	bestHyperCubes.resize(slice::graph.size() + 2);
	
	// Information matrix
	path_info_matrix paths_info;

	// For each starting slice (with default configs)
	for (unsigned start = 0; start < slice::graph.size(); start++)
	{
		// Reference, for brevity
		const unsigned numVertices = slice::lookup(start).numVertices;
		
		// Set the size of the 1-wide rectangle with just 1 column.
		paths_info.lengths[1].info[start].num_induced = numVertices;
		
		// Set the starting vertex
		paths_info.lengths[1].info[start].start = start;
		
		// Check to see if this is the best 1-tall hypercube
		if (bestHyperCubes[1].density.num < numVertices)
		{
			bestHyperCubes[1] =
				extractHyperCube(paths_info, 1, start, fraction(numVertices,1));
		}
	}
	
	// For each length
	for (unsigned len = 2; len < paths_info.lengths.size(); len++)
	{
		if (trace) std::cout << "length = " << len << ":" << std::endl;
		
		// End vertex of the stack with the most vertices
		unsigned bestEndVertex = 0;
		
		// Try to expand each cell that has a valid path
		for (unsigned end = 0; end < slice::graph.size(); end++)
		{
			auto [old_num_induced,dummy,start] =
				paths_info.lengths[len - 1].info[end];
			
			// Expand in every possible way
			for (unsigned adj : slice::graph[end].adjList)
			{
				// Reference, for brevity
				path_info& new_info = paths_info.lengths[len].info[adj];
				
				// Number of vertices that are added with the new slice
				unsigned more_vertices = slice::lookup(adj).numVertices;
				
				if (new_info.num_induced < old_num_induced + more_vertices)
				{
					new_info = path_info(old_num_induced + more_vertices, end, start);
					
					// Check to see if this is the best tile of this given length
					unsigned newNumVertices = paths_info.lengths[len].info[adj].num_induced;
					
					if (newNumVertices > paths_info.lengths[len].info[bestEndVertex].num_induced)
					{
						bestEndVertex = adj;
					}
					
					// A cycle has been found, check to see if it is the new best
					if (adj == start)
					{
						fraction density(paths_info.lengths[len - 1].info[end].num_induced,
							n*n*(len - 1));
						
						if (density > bestTiling.density)
						{
							bestTiling = extractHyperCube(paths_info, len - 1,
								end, density);
							
							std::cout << "found: " << density << std::endl;
							
							for (unsigned vertex : bestTiling.slices.path)
							{
								unsigned sliceNum = slice::graph[vertex].sliceNum;
								
								std::cout << slice::slices[sliceNum] <<
									er_store[slice::graph[vertex].erID]
									<< std::endl << std::endl;
							}
						}
					}
				}
			}
		}
		
		if (trace)
		{
			std::cout << "best hypercube has " << paths_info.lengths[len].info[bestEndVertex].num_induced
				<< " vertices" << std::endl;
			
			/*
			// Cleanup of a few random print statements, not working yet
			
			fraction density(paths_info.lengths[len].info[bestEndVertex].num_induced,
				n*n*len);
			
			auto bestCube = extractHyperCube(paths_info, len, adj, density);
			
			const auto& len_info = paths_info.lengths[len];
			
			for (const auto& end_info : len_info.info)
			{
				std::cout << "(" << end_info.num_induced << ","
					<< end_info.second_to_last << ") ";
			}
			std::cout << std::endl;
			*/
		}
	}
}

int main(int argn, char** args)
{
	auto start_time = clock();
	
	parseArgs(argn, args);
	
	produceColumns();
	
	fillInColumnAdjLists();
	
	produceSlices();
	
	std::cout << slice::slices.size() << " physical slices" << std::endl;
	std::cout << "Generated in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
	
	fillInSliceAdjLists();
	
	// Print some information about what can come before a given vertex,
	// and in what configurations. This is just to get an idea of how many
	// extraneous edges exist.
	/*
	unsigned v = 2;
	
	std::cout << "Vertex " << v << ":" << std::endl;
	std::cout << slice::lookup(v) << std::endl;
	for (const auto& config : slice::lookup(v).configs)
	{
		std::cout << config.vertexID << ": " << er_store[config.erID] << std::endl;
	}
	for (unsigned i = 0; i < slice::graph.size(); i++)
	{
		std::cout << i << ':';
		for (const auto& adj : slice::graph[i].adjList)
		{
			if (slice::graph[adj.first].sliceNum == v)
			{
				std::cout << ' ' << adj.first;
			}
		}
		std::cout << std::endl;
	}
	*/
	
	std::cout << slice::graph.size() << " slice configurations" << std::endl;
	std::cout << "Adjacency lists filled in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
	std::cout << "Enumerating tiles" << std::endl << std::endl;
	
	enumerate();
	
	std::cout << std::endl << "Finished in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
	
	// The log gets a bit cluttered with tracing, so print this again. This would be recent without tracing.
	if (trace)
	{
		std::cout << "Best tiling (rotations ignored):" << std::endl;
		std::cout << "Density = " << bestTiling.density << std::endl;
		for (unsigned vertex : bestTiling.slices.path)
		{
			unsigned sliceNum = slice::graph[vertex].sliceNum;
			std::cout << slice::slices[sliceNum];
			std::cout << er_store[slice::graph[vertex].erID]
				<< std::endl << std::endl;
		}
	}
}
