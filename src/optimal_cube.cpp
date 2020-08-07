#include <cstdlib>
#include <iostream>
#include <vector>
#include <compare>
#include <list>
#include <ctime>
#include <array>
#include "equivRelation.hpp"
#include "fraction.hpp"

// The current state of this program is, frankly, awful, and I apologize
// profusely. Right now I just need this to work, I will work on making
// it prettier later.

// ==========================================================
// Global variables, struct definitions, and printing functions.

unsigned n;

bool trace = false;

typedef char componentNumType;

struct configuration
{
	equivRelation er;
	unsigned vertexID;
	
	configuration(equivRelation e, unsigned vID) :
		er(e), vertexID(vID) {}
};

// The physical configuration of the column.
// Stored as an integer, the least significant
// n bits represent the column, with a 1 being
// filled in, and a 0 being empty.
// For example, XX_XX_X would be 1101101

struct physicalColumn
{
	// Assumes number of used bits is n.
	// Contains a component number, or EMPTY or COMPLETELY_EMPTY
	constexpr static int EMPTY = -1;
	constexpr static int COMPLETELY_EMPTY = -2;
	
	std::vector<componentNumType> componentNums;
	
	unsigned numComponents;
	unsigned numVertices;
	
	std::vector<configuration> configs;
	
	physicalColumn(unsigned p, unsigned vID) : componentNums(n), numComponents(0),
		numVertices(0), configs()
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
		
		// Emplace the first config, which is with all elements in the ER non-equivalent.
		configs.emplace_back(equivRelation(numComponents), vID);
	}
	
	bool operator[](unsigned i) const { return componentNums[i] >= 0; }
};

/*
A vertex in the slice(column) graph. Contains info about which column it represents.
*/

struct vertex
{
	std::vector<unsigned> adjList;
	
	unsigned sliceNum;
	unsigned configNum;
	
	vertex(unsigned sn, unsigned cn) :
		sliceNum(sn), configNum(cn) {}
};

struct vertexWithSymmetries
{
	// first is the vertex number, second is the number of the first symmetry
	// that can follow the slice.
	std::vector<std::pair<unsigned,unsigned>> adjList;
	
	unsigned sliceNum;
	unsigned configNum;
	
	vertexWithSymmetries(unsigned sn, unsigned cn) :
		sliceNum(sn), configNum(cn) {}
};

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

std::vector<physicalColumn> columns;

std::vector<vertex> column_graph;

std::ostream& operator<<(std::ostream& stream, physicalColumn p)
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
		stream << columns[column_graph[c].sliceNum] << std::endl;
	}
	
	return stream;
}

struct slice
{
	// Outer index is the permutation, there are at most 8 of these.
	// Uses the standard form of coord1 * n + coord2 for the index.
	std::vector<std::vector<componentNumType>> componentNums;
	
	unsigned numComponents;
	unsigned numVertices;
	
	std::vector<configuration> configs;

	slice(const pathWithoutSymmetries& p, unsigned vID) : numVertices(p.numVertices)
	{
		componentNums.emplace_back(p.path.size() * n);
	
		equivRelation combination;
		
		// Append all of the configurations of each component column in succession.
		for (unsigned vID : p.path)
		{
			const auto& vert = column_graph[vID];
			
			// Does this need to be this specific config, or will a default one work?
			// It seems that this would produce the same result.
			combination += columns[vert.sliceNum].configs[vert.configNum].er;
		}
		
		// Total offset before both columns, and offset between them.
		unsigned base_offset = 0, offset;
		
		// Iterate over each pair of adjacent columns
		for (unsigned i = 0; i < p.path.size() - 1; i++)
		{	
			const auto& col1 = columns[column_graph[p.path[i]].sliceNum];
			const auto& col2 = columns[column_graph[p.path[i + 1]].sliceNum];
			
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
			const auto& col = columns[column_graph[p.path[i]].sliceNum];
		
			for (unsigned j = 0; j < n; j++)
			{
				componentNums[0][pos++] = col[j]
					? cgl[col.componentNums[j] + base_offset]
					: -1;
			}
			
			base_offset += col.numComponents;
		}
		
		// Emplace the first config, which is with all elements in the ER non-equivalent.
		configs.emplace_back(equivRelation(numComponents), vID);
	}
};

std::ostream& operator<<(std::ostream& stream, const slice& s)
{
	for (unsigned i = 0; i < n; i++)
	{
		for (unsigned j = 0; j < n; j++)
		{
			std::cout << ((s.componentNums.front()[i * n + j] >= 0) ? 'X' : '_') << ' ';
		}
		
		std::cout << "| ";
		
		for (unsigned j = 0; j < n; j++)
		{
			if (s.componentNums.front()[i * n + j] >= 0)
			{
				std::cout << (int)s.componentNums.front()[i * n + j] << ' ';
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

std::vector<slice> slices;

std::vector<vertexWithSymmetries> slice_graph;

// =======================================================================
// Various functions

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
	
	columns.reserve(numCols);
	
	for (unsigned i = 0; i < numCols; i++)
	{
		columns.emplace_back(i,i);
		
		if (trace)
		{
			std::cout << i << ": " << columns[i] << std::endl;
		}
	}
}

/*
If the physical column numbered 'physColNo' can succeed the vertex
numbered 'vID', returns true and places into 'result' the equivalence
relation that the successor would need to be configured with.
Otherwise, returns false.
*/

bool column_succeeds(unsigned vID, unsigned physColNo, equivRelation& result)
{
	// Reference these, for brevity
	const physicalColumn& after = columns[physColNo];
	const physicalColumn& before = columns[column_graph[vID].sliceNum];
	
	if (trace) std::cout << "    " << after;
	
	const equivRelation& beforeConfig = before.configs[column_graph[vID].configNum].er;

	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	const unsigned offset = after.numComponents;
	
	equivRelation combination = equivRelation(offset).append(beforeConfig);
	
	for (unsigned i = 0; i < n; i++)
	{
		// If both vertices exist
		if (after[i] && before[i])
		{
			// Check if their associated equivalence classes
			// are already equivalent. If so, this would
			// cause a cycle, so return false.
			
			if (combination.equivalent(after.componentNums[i],
				offset + before.componentNums[i]))
			{
				return false;
			}
			
			// Otherwise, merge them.
			combination.merge(after.componentNums[i], offset + before.componentNums[i]);
		}
	}
	
	// Result is acyclic, so produce the ER that should be used
	// by shaving off the last 'before.numComponents' items.
	
	result = combination.shave(before.numComponents);
	
	return true;
}

void fillInColumnAdjLists()
{
	// Start by inserting the first 2^n vertices
	for (unsigned i = 0; i < columns.size(); i++)
	{
		column_graph.emplace_back(i,0);
	}
	
	// Out-parameter for 'succeeds' function calls
	equivRelation result;
	
	// Fill in each adjacency list. column_graph.size() may change with
	// each iteration, but will eventually stop growing.
	for (unsigned vID = 0; vID < column_graph.size(); vID++)
	{
		/*
		// The 'before' physical column
		const auto& col = columns[column_graph[vID].sliceNum];
		const unsigned configNo = column_graph[vID].configNum;
		
		if (trace)
		{
			std::cout << col << ", config = " <<
				col.configs[configNo].er << ":" << std::endl;
		}
		*/
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < columns.size(); i++)
		{
			if (!column_succeeds(vID,i,result))
			{
				if (trace) std::cout << " false" << std::endl;
				continue;
			}
			
			if (trace) std::cout << " true, config = " << result << std::endl;
			
			// Search for the result in the 'after' physical column's configs
			bool found = false;
			for (unsigned j = 0; j < columns[i].configs.size(); j++)
			{
				if (columns[i].configs[j].er == result)
				{
					found = true;
					column_graph[vID].adjList.push_back(columns[i].configs[j].vertexID);
					break;
				}
			}
			
			// Need to make a new config and vertex, and add to the adjacency list.
			if (!found)
			{
				columns[i].configs.emplace_back(result, column_graph.size());
				column_graph.emplace_back(i,columns[i].configs.size() - 1);
				
				// Add the adjacency, which is now the last vertex.
				column_graph[vID].adjList.push_back(column_graph.size() - 1);
			}
		}
	}
	
	if (trace)
	{
		for (unsigned i = 0; i < column_graph.size(); i++)
		{
			std::cout << i << ':';
			
			for (unsigned neighbor : column_graph[i].adjList)
			{
				std::cout << ' ' << neighbor;
			}
			
			std::cout << std::endl;
		}
		
		std::cout << std::endl;
	}
}

bool prune(pathWithoutSymmetries& p)
{
	// Prune any slice with a non-induced vertex with no induced neighbors.
	// Look for 'completely empty' vertices.
	
	if (p.path.size() == 2) // Check first column
	{
		const auto& prev1 = columns[column_graph[p.path[1]].sliceNum].componentNums;
		const auto& prev2 = columns[column_graph[p.path[0]].sliceNum].componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev2[i] == physicalColumn::COMPLETELY_EMPTY &&
				prev1[i] < 0)
			{
				return true;
			}
		}
	}
	else if (p.path.size() > 2) // Check any middle column
	{
		const auto& prev1 =
			columns[column_graph[p.path[p.path.size()-1]].sliceNum].componentNums;
		const auto& prev2 =
			columns[column_graph[p.path[p.path.size()-2]].sliceNum].componentNums;
		const auto& prev3 =
			columns[column_graph[p.path[p.path.size()-3]].sliceNum].componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev2[i] == physicalColumn::COMPLETELY_EMPTY &&
				prev1[i] < 0 && prev3[i] < 0)
			{
				return true;
			}
		}
	}
	
	if (p.path.size() == n) // Check last column
	{
		const auto& prev1 = columns[column_graph[p.path[n-1]].sliceNum].componentNums;
		const auto& prev2 = columns[column_graph[p.path[n-2]].sliceNum].componentNums;
		
		for (unsigned i = 0; i < n; i++)
		{
			if (prev1[i] == physicalColumn::COMPLETELY_EMPTY &&
				prev2[i] < 0)
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
		slices.emplace_back(p,slices.size());
		
		// Reference, for brevity
		auto& componentNums = slices.back().componentNums;
		
		// Produce each symmetry. Should one of them be lexicographically smaller
		// than this one, then remove it. Start at index 1, since 0 is the identity.
		for (unsigned i = 1; i < perms.size(); i++)
		{
			componentNums.emplace_back(applyPermutation(i, componentNums[0]));
			
			// If this new symmetry is lexicographically smaller than the original,
			// then we can prune this one.
			if (compareSymmetries(componentNums.back(), componentNums[0]) < 0)
			{
				slices.pop_back();
				return;
			}
			
			// Otherwise, ensure there are no duplicate symmetries
			for (unsigned j = 0; j < componentNums.size() - 1; j++)
			{
				if (compareSymmetries(componentNums.back(), componentNums[j]) == 0)
				{
					componentNums.pop_back();
					break;
				}
			}
			
			std::cout.flush();
		}
		
		if (trace)
		{
			std::cout << p;
			
			for (auto sym : componentNums)
			{
				for (auto i : sym)
				{
					std::cout << (int)i << ' ';
				}
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		
		return;
	}
	
	// Iterate using each available adjacent column.
	for (unsigned adj : column_graph[p.path.back()].adjList)
	{
		unsigned numVertices = columns[column_graph[adj].sliceNum].numVertices;
	
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
		p.numVertices = columns[i].numVertices;
		
		produceSlicesRecursive(p);
	}
}

// symmetryNum identifies which index in componentNums to use. (which symmetry)
bool slice_succeeds(unsigned vID, unsigned sliceNum, unsigned symmetryNum,
	equivRelation& result)
{
	// Reference these, for brevity
	const slice& after = slices[sliceNum];
	const slice& before = slices[slice_graph[vID].sliceNum];
	
	// before uses the base configuration, always.
	const auto& afterCN = after.componentNums[symmetryNum];
	const auto& beforeCN = before.componentNums.front();
	
	const equivRelation& beforeConfig = before.configs[slice_graph[vID].configNum].er;

	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	const unsigned offset = after.numComponents;
	
	equivRelation combination = equivRelation(offset).append(beforeConfig);
	
	for (unsigned i = 0; i < afterCN.size(); i++)
	{
		// If both vertices exist
		if (afterCN[i] >= 0 && beforeCN[i] >= 0)
		{
			// Check if their associated equivalence classes
			// are already equivalent. If so, this would
			// cause a cycle, so return false.
			
			if (combination.equivalent(afterCN[i], offset + beforeCN[i]))
			{
				return false;
			}
			
			// Otherwise, merge them.
			combination.merge(afterCN[i], offset + beforeCN[i]);
		}
	}
	
	// Result is acyclic, so produce the ER that should be used
	// by shaving off the last 'before.numComponents' items.
	
	result = combination.shave(before.numComponents);
	
	return true;
}

// Returns true iff v's adjacency list contains neighbor.
bool contains(const vertexWithSymmetries& v, unsigned neighbor)
{
	for (const auto& adj : v.adjList)
	{
		if (adj.first == neighbor) return true;
	}
	return false;
}

void fillInSliceAdjLists()
{
	// Start by inserting the first 2^n vertices
	for (unsigned i = 0; i < slices.size(); i++)
	{
		slice_graph.emplace_back(i,0);
	}
	
	// Out-parameter for 'succeeds' function calls
	equivRelation result;
	
	// Fill in each adjacency list. slice_graph.size() may change with
	// each iteration, but will eventually stop growing.
	for (unsigned vID = 0; vID < slice_graph.size(); vID++)
	{
		// Go through each of the physical columns (as the afters),
		// and see if it can succeed this configuration.
		for (unsigned i = 0; i < slices.size(); i++)
		{
			// Go through each symmetry
			unsigned symNum = 0;
			for (; symNum < slices[i].componentNums.size(); symNum++)
			{
				if (slice_succeeds(vID,i,symNum,result))
				{
					// This is in this block so that multiple configurations of a slice
					// can be added to the adjacency list.
					
					// TODO: Exclude configs that are 'supersets'
					// of other configs (and prove this is valid).
					
					// TODO: Find a way to avoid duplicates that is more efficient than
					// doing a full scan of the vector each time you add something.
					
					// Search for the result in the 'after' physical column's configs
					bool found = false;
					for (const auto& config : slices[i].configs)
					{
						if (config.er == result)
						{
							found = true;
							
							if (!contains(slice_graph[vID], config.vertexID))
							{
								slice_graph[vID].adjList.emplace_back(
									config.vertexID, symNum
								);
							}
							break;
						}
					}
					
					// Need to make a new config and vertex, and add to the adjacency list.
					if (!found)
					{
						slices[i].configs.emplace_back(result, slice_graph.size());
						slice_graph.emplace_back(i,slices[i].configs.size() - 1);
						
						// Add the adjacency, which is now the last vertex.
						if (!contains(slice_graph[vID], slice_graph.size() - 1))
						{
							slice_graph[vID].adjList.emplace_back(
								slice_graph.size() - 1, symNum
							);
						}
					}
				}
			}
		}
	}
	
	if (trace)
	{
		for (unsigned i = 0; i < slice_graph.size(); i++)
		{
			std::cout << i << ':';
			
			for (auto neighbor : slice_graph[i].adjList)
			{
				std::cout << ' ' << neighbor.first;
			}
			
			std::cout << std::endl;
		}
		
		std::cout << std::endl;
	}
}

struct path_info
{
	// -1 for -inf
	int num_induced;
	
	// The vertex that precedes this one in the optimal path.
	unsigned second_to_last;
	
	path_info() : num_induced(-1), second_to_last(0) {}
	
	path_info(int n_i, unsigned s_t_l) :
		num_induced(n_i), second_to_last(s_t_l) {}
};

// All path info for one length and starting vertex.
// .info[s] gives the information about the path from
// the predetermined start column to s, of a predetermined length.
struct path_info_vector
{
	std::vector<path_info> info;
	
	path_info_vector() : info(slice_graph.size()) {}
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
	path_info_matrix() : lengths(slice_graph.size() + 2) {}
};

hyperCube extractHyperCube(path_info_matrix paths_info, const unsigned len,
	unsigned end, fraction density)
{
	hyperCube h;
	
	unsigned currentVertex = end;
	for (unsigned length = len; length > 0; length--)
	{
		h.slices.path.push_back(currentVertex);
		
		currentVertex = paths_info.lengths[length].info[currentVertex].second_to_last;
	}
	
	// Since the items that should be pushed to the front are pushed to the back,
	// reverse the vector.
	std::reverse(h.slices.path.begin(),h.slices.path.end());
	
	h.density = density;
	
	return h;
}

// Helper for enumerate, does enumeration for a given start vertex.
void enumerateWithStart(unsigned start)
{
	// Reference, for brevity
	//const slice& slic = columns[start].cols[0];
	const unsigned numVertices = slices[slice_graph[start].sliceNum].numVertices;
	
	if (trace) std::cout << "Starting with vertex " << start << std::endl;
	
	// Information matrix
	path_info_matrix paths_info;
	
	// Set the size of the 1-wide rectangle with just 1 column.
	paths_info.lengths[1].info[start].num_induced = numVertices;
	
	// This 
		
	// For each length
	for (unsigned len = 2; len < paths_info.lengths.size(); len++)
	{
		// Try to expand each cell that has a valid path
		for (unsigned end = 0; end < slice_graph.size(); end++)
		{
			// Reference, for brevity
			const int& old_num_induced = paths_info.lengths[len - 1].info[end].num_induced;
			
			// Ignore cells with no valid path
			if (old_num_induced == -1) continue;
			
			// Expand in every possible way
			for (const auto& neighbor : slice_graph[end].adjList)
			{
				unsigned adj = neighbor.first;
				
				// Reference, for brevity
				path_info& new_info = paths_info.lengths[len].info[adj];
				
				// Number of vertices that are added with the new slice
				int more_vertices = slices[slice_graph[adj].sliceNum].numVertices;
				
				if (new_info.num_induced < old_num_induced + more_vertices)
				{
					new_info = path_info(old_num_induced + more_vertices, end);
					
					// Check to see if this is the best tile of this given length
					fraction density(paths_info.lengths[len].info[adj].num_induced, n*n*len);
					
					if (density > bestHyperCubes[len].density)
					{
						bestHyperCubes[len] = extractHyperCube(paths_info, len, adj, density);
					}
					
					// A cycle has been found, check to see if it is the new best
					if (adj == start)
					{
						fraction tile_density(paths_info.lengths[len - 1].info[end].num_induced,
							n*n*(len - 1));
						
						if (tile_density > bestTiling.density)
						{
							bestTiling = extractHyperCube(paths_info, len - 1,
								end, tile_density);
							
							std::cout << "\rfound: " << bestTiling.density;
						}
					}
				}
			}
		}
	}
	/*
	if (trace)
	{
		for (unsigned len = 1; len < paths_info.lengths.size(); len++)
		{
			const auto& len_info = paths_info.lengths[len];
			
			//for (unsigned end = 0; end < slice_graph.size(); end++)
			for (const auto& end_info : len_info.info)
			{
				if (end_info.num_induced == -1)
				{
					std::cout << "X ";
				}
				else
				{
					std::cout << "(" << end_info.num_induced << ","
						<< end_info.second_to_last << ") ";
				}
			}
			std::cout << std::endl;
		}
		std::cout << std::endl;
	}
	*/
}

/*------------------------------------
Prints the maximum size and density of
every rectangle sized 1xn through nxn.
Also, prints the maximum density tile.
------------------------------------*/

void enumerate()
{
	bestHyperCubes.resize(slice_graph.size() + 1);

	// For each starting slice (with default configs)
	for (unsigned start = 0; start < slices.size(); start++)
	{
		enumerateWithStart(start);
	}
}

int main(int argn, char** args)
{
	auto start_time = clock();
	
	parseArgs(argn, args);
	
	produceColumns();
	
	fillInColumnAdjLists();
	
	produceSlices();
	
	std::cout << slices.size() << " physical slices" << std::endl;
	
	fillInSliceAdjLists();
	
	std::cout << slice_graph.size() << " slice configurations" << std::endl;
	std::cout << "Enumerating tiles" << std::endl;
	
	enumerate();
	// Without tracing, overwrite the running line of tile densities
	if (!trace) std::cout << '\r';
	
	std::cout << "Finished in " << (float)(clock()-start_time)/(CLOCKS_PER_SEC)
		<< " seconds" << std::endl;
	
	std::cout << "Best hypercube sizes:" << std::endl;
	for (unsigned len = 1; len < bestHyperCubes.size(); len++)
	{
		std::cout << len << ": " << bestHyperCubes[len].density.num << std::endl;
	}
	
	std::cout << "Best tiling (rotations ignored):" << std::endl;
	std::cout << "Density = " << bestTiling.density << std::endl;
	for (unsigned vertex : bestTiling.slices.path)
	{
		unsigned sliceNum = slice_graph[vertex].sliceNum;
		std::cout << slices[sliceNum];
		std::cout << slices[sliceNum].configs[slice_graph[vertex].configNum].er
			<< std::endl << std::endl;
	}
}
