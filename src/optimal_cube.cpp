#include <cstdlib>
#include <iostream>
#include <vector>
#include <compare>
#include <list>
#include "equivRelation.hpp"
#include "fraction.hpp"

// ==========================================================
// Global variables and struct definitions

unsigned n;

std::vector<std::vector<equivRelation>> erConfigs;

bool trace = false;

struct columnClass;
std::vector<columnClass> columns;

// The physical configuration of the column.
// Stored as an integer, the least significant
// n bits represent the column, with a 1 being
// filled in, and a 0 being empty.
// For example, XX_XX_X would be 1101101

struct physicalColumn
{
	// Assumes number of used bits is n.
	unsigned phys;
	
	physicalColumn() : phys(0) {}
	
	physicalColumn(unsigned p) : phys(p) {}
	
	bool operator[](unsigned i) const { return (phys >> i) & 1; }
	
	// Returns the bitwise reversal of this.
	physicalColumn reverse() const
	{
		unsigned result = 0;
		
		for (unsigned i = 0; i < n; i++)
		{
			// For each 1, put a 1 in the opposite position in result.
			if ((1 << i) & phys)
				result |= (1 << (n - i - 1));
		}
		
		return physicalColumn(result);
	}
	
	physicalColumn& operator++()
	{
		++phys;
		
		return *this;
	}
	
	bool operator==(const physicalColumn& other) const = default;
	std::strong_ordering operator<=>(const physicalColumn& other) const = default;
	
	unsigned numComponents() const
	{
		bool lastWasSpace = true;
		unsigned numComp = 0;
		for (unsigned i = 0; i < n; i++)
		{
			bool currentIsVertex = (*this)[i];
		
			if (currentIsVertex && lastWasSpace)
			{
				numComp++;
			}
			
			lastWasSpace = !currentIsVertex;
		}
		
		return numComp;
	}
	
	unsigned numVertices() const
	{
		unsigned numVert = 0;
		for (unsigned i = 0; i < n; i++)
		{
			if ((*this)[i]) numVert++;
		}
		
		return numVert;
	}
};

struct column
{
	physicalColumn physical;
	
	// The number of vertices in the column.
	// For example, XX_XX_X would have 5.
	unsigned numVertices;
	
	// The number of connected components in the column.
	// For example, XX_XX_X would have 3.
	unsigned numComponents;
	
	// Which of the ER configurations is this column using.
	// For example, XX_XX_X could have any of:
	// 0 0 0
	// 0 0 1
	// 0 1 0
	// 0 1 1
	// 0 1 2
	unsigned erConfig;
	
	column(physicalColumn p) : column(p,0) {}
	
	column(physicalColumn p, unsigned configNo) :
		physical(p), numVertices(p.numVertices()),
		numComponents(p.numComponents()), erConfig(configNo) {}
};

struct columnClass
{
	std::vector<column> cols;
	
	// The first item in the pair is the class that can come after it,
	// the second item is the first member of that class that can come
	// after it.
	std::list<std::pair<unsigned,unsigned>> adjList;
	
	std::strong_ordering operator<=>(const columnClass& other)
	{
		// Compare number of vertices. We want a larger number of vertices
		// to evaluate as smaller, so swap the order of the comparison.
		return other.cols.front().numVertices <=> cols.front().numVertices;
	}
};

struct rectangle
{
	std::list<std::pair<unsigned,unsigned>> columnPath;
	fraction density;
	
	std::strong_ordering operator<=>(const rectangle& other) const
	{
		auto compare = density <=> other.density;
		
		// If they are different, return the result of the comparison.
		if (compare != 0) return compare;
		
		// Otherwise, prefer shorter tilings over longer ones.
		return other.columnPath.size() <=> columnPath.size();
	}
	
	rectangle() : columnPath(), density() {}
	
	void append(unsigned col_class, unsigned col_no)
	{
		columnPath.emplace_back(col_class, col_no);
		density.den += n;
		density.num += columns[col_class].cols[0].numVertices;
	}
};

struct tile
{
	std::list<std::pair<unsigned,unsigned>> columnPath;
	unsigned numVertices;
	
	std::strong_ordering operator<=>(const tile& other) const
	{
		if (other.columnPath.empty()) return numVertices <=> 0;
		else if (columnPath.empty()) return 0 <=> other.numVertices;
		
		auto compare = (numVertices * other.columnPath.size())
			<=> (other.numVertices * columnPath.size());
		
		// If they are different, return the result of the comparison.
		if (compare != 0) return compare;
		
		// Otherwise, prefer shorter tilings over larger ones.
		return other.numVertices <=> numVertices;
	}
	
	tile() : columnPath(), numVertices(0) {}
	
	tile(unsigned i)
	{
		columnPath.push_back(std::pair(i,0));
		numVertices = columns[i].cols.front().numVertices;
	}
};

std::vector<rectangle> bestRectangles;
rectangle bestTile;

// =======================================================================
// Various functions

std::ostream& operator<<(std::ostream& stream, physicalColumn p)
{
	for (unsigned i = 0; i < n; i++)
	{
		stream << (p[i] ? 'X' : '_');
	}
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const column& c)
{
	stream << c.physical;
	
	stream << ", config: " << erConfigs[c.numComponents][c.erConfig];
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const rectangle& t)
{
	stream << t.density << std::endl;
	
	if (t.density.den/n > 20)
	{
		return (stream << "(tile omitted for brevity)" << std::endl);
	}
	
	bool reverse = false;
	for (auto& c : t.columnPath)
	{
		if (c.second == 1) reverse = !reverse;
		
		//stream << c.first << ": ";
		if (!reverse) stream << columns[c.first].cols.front().physical;
		else stream << columns[c.first].cols.back().physical;
		
		stream << std::endl;
	}
	
	return stream;
}

std::ostream& operator<<(std::ostream& stream, const tile& t)
{
	bool reverse = false;
	for (auto& c : t.columnPath)
	{
		if (c.second == 1) reverse = !reverse;
		
		if (!reverse) stream << columns[c.first].cols.front().physical;
		else stream << columns[c.first].cols.back().physical;
		
		stream << std::endl;
	}
	
	return stream;
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

// Fills in erConfigs
void produceConfigs()
{
	// The largest number of connected components can be the ceiling
	// of half of n.
	unsigned ceilHalf = (n + 1) / 2;
	
	// Ignore index 0
	erConfigs.resize(ceilHalf + 1);
	
	for (unsigned i = 1; i <= ceilHalf; i++)
	{
		erConfigs[i] = equivRelation::enumerate(i);
		
		if (trace)
		{
			std::cout << "Configs of size " << i << ":" << std::endl;
			for (auto& er : erConfigs[i])
			{
				std::cout << er << std::endl;
			}
		}
	}
}

/*--------------------------------------------------------
Returns true iff physical contains one of the
pruning patterns: (interpret as a bit sequence,
	0 is _, 1 is X, direction doesn't matter)
___ (3 empty spaces in a row)
__X__ (a block surrounded by 2 empty spaces on both sides)

Can use a DFA for this:

State 0: 'X' (less than 2 spaces prior)
_ -> 1, X -> 0

State 1: '_' (exactly one space), has an X before
  it, but no more than one space behind that)
  (start state)
_ -> 2, X -> 0

State 2: '__' (exactly two spaces, X before,
  no more than one space behind that)
  (accepts if ending in this state)
_ -> 5, X -> 3

State 3: '__X'
_ -> 4, X -> 0

State 4: '__X_' (accepts if ending in this state)
_ -> 5, X -> 0

State 5: Accept (either ___ or __X__ has been seen)
--------------------------------------------------------*/

bool prune(physicalColumn physical)
{
	// Start state
	unsigned state = 1;
	
	// Generated from the transitions given above
	constexpr static unsigned transition[] {1,0,2,0,5,3,4,0,5,0};
	
	for (unsigned i = 0; i < n; i++)
	{
		// This formula gives us a unique case for each input:
		unsigned check = (2 * state) + physical[i];
		
		state = transition[check];
		
		// Short circuit the 'live' state
		if (state == 5) return true;
	}
	
	// 2 and 4 are the accept states
	return state == 2 || state == 4;
}

// This does lead to some inefficiencies that likely could be
// avoided, but it's simple enough, and the true performance
// bottleneck will be elsewhere.
unsigned findER(unsigned numComp, const equivRelation& ER)
{
	for (unsigned i = 0; i < erConfigs[numComp].size(); i++)
	{
		if (erConfigs[numComp][i] == ER) return i;
	}
	
	// This shouldn't happen, but just to make the compiler happy.
	return 0;
}

void produceColumns()
{
	physicalColumn maxPhysical((1 << n) - 1);
	unsigned numPruned = 0;
	for (physicalColumn physical; physical <= maxPhysical; ++physical)
	{
		// Rules for pruning physical columns, use the pruning rules
		// plus use the smaller of a column and its reverse.
		physicalColumn rev = physical.reverse();
		if (!prune(physical) && (physical <= rev))
		{
			column c(physical);
			for (unsigned configNo = 0; configNo < erConfigs[c.numComponents].size(); configNo++)
			{
				// Rules for pruning ERs, use if either physical is strictly less than
				// its reverse or if the ER is at most its reverse
				auto& ER = erConfigs[c.numComponents][configNo];
				auto ERrev = ER.reverse();
				if (physical < rev || ER <= ERrev)
				{
					columns.emplace_back();
					
					// std::vector<column>, a 'class' of columns
					auto& colClass = columns.back();
					
					colClass.cols.emplace_back(physical, configNo);
					
					if (physical < rev || ER < ERrev)
						colClass.cols.emplace_back(rev, findER(c.numComponents,ERrev));
				}
			}
		}
		else numPruned++;
	}
	
	if (trace)
	{
		std::cout << columns.size() << " columns total" << std::endl;
		std::cout << (maxPhysical.phys + 1 - numPruned) << " physical columns used" << std::endl;
		std::cout << numPruned << " physical columns pruned" << std::endl;
	
		for (unsigned i = 0; i < columns.size(); i++)
		{
			std::cout << "Column class " << i << ":" << std::endl;
			
			for (unsigned j = 0; j < columns[i].cols.size(); j++)
			{
				std::cout << "    " << columns[i].cols[j] << std::endl;
			}
		}
	}
}

// Sorts column class array by putting the classes with the most vertices in
// front, so the algorithm will greedily choose those over smaller ones.
void sortGreedy()
{
	// Need to use stable_sort so that classes with identical physical forms
	// stay together, for use in fillInAdjLists().
	std::stable_sort(columns.begin(), columns.end());
}

/*
If 'after' can succeed 'before', returns true
and places into 'result' the equivalence relation that the successor would
need to be configured with. Otherwise, returns false.
*/

bool succeeds(const column& before, physicalColumn after, equivRelation& result)
{
	unsigned numComponents = after.numComponents();
	
	// The equivalence relation corresponding to 'after' is the first
	// part of the combined ER, the one corresponding to 'before' is
	// the second part.
	
	equivRelation combination = equivRelation(numComponents)
		.append(erConfigs[before.numComponents][before.erConfig]);
	
	// These variables are prefixed with 'before' and 'after' to
	// denote which column they are associated with
	
	// Component identifiers, beforeCN starts at numComponents due to
	// it being the second part of the combined ER.
	unsigned beforeComponentNum = numComponents - 1, afterComponentNum = (unsigned)(-1);
	bool beforeLastWasSpace = true, afterLastWasSpace = true;
	
	for (unsigned i = 0; i < n; i++)
	{
		bool beforeCurrentIsVertex = before.physical[i];
		bool afterCurrentIsVertex = after[i];
		
		// Update component numbers. These get updated on the 'leading edge'
		// of the component, which explains why the component numbers start
		// at 1 less then their respective first component numbers.
		if (beforeCurrentIsVertex && beforeLastWasSpace)
		{
			beforeComponentNum++;
		}
		if (afterCurrentIsVertex && afterLastWasSpace)
		{
			afterComponentNum++;
		}
		
		beforeLastWasSpace = !beforeCurrentIsVertex;
		afterLastWasSpace = !afterCurrentIsVertex;
		
		// If both vertices exist
		if (beforeCurrentIsVertex && afterCurrentIsVertex)
		{
			// Check if their associated equivalence classes
			// are already equivalent. If so, this would
			// cause a cycle, so return false.
			
			if (combination.equivalent(beforeComponentNum, afterComponentNum))
			{
				return false;
			}
			
			// Otherwise, merge them.
			combination.merge(beforeComponentNum, afterComponentNum);
		}
	}
	
	// Result is acyclic, so produce the ER that should be used
	// by shaving off the last 'before.numComponents' items.
	
	result = combination.shave(before.numComponents);
	
	return true;
}

void fillInAdjLists()
{
	for (unsigned i = 0; i < columns.size(); i++)
	{
		column before = columns[i].cols.front();
		
		if (trace) std::cout << "Before: " << before << std::endl;
		
		for (unsigned j = 0; j < columns.size(); )
		{
			physicalColumn after = columns[j].cols.front().physical;
			
			if (trace) std::cout << " After: " << after << ": ";
			
			physicalColumn rev = after.reverse();
			equivRelation result;
			
			if (succeeds(before, after, result))
			{
				if (trace)
					std::cout << "yes, uses normal form, config: " << result << std::endl;
				unsigned configNo = findER(columns[j].cols.front().numComponents, result);
			
				if (after == rev)
				{
					// All columns in the class should be searched for
					// the ER required
					
					// Note that in all of these searches, the result is
					// guaranteed to be found, so we don't need to check indexes.
					
					bool cont = true;
					while (true)
					{
						// Look through each item in the current list
						for (unsigned k = 0; k < columns[j].cols.size(); k++)
						{
							if (columns[j].cols[k].erConfig == configNo)
							{
								columns[i].adjList.emplace_back(j,k);
								
								cont = false;
								break;
							}
						}
						
						if (!cont) break;
						
						// Not in this class, try the next one.
						j++;
					}
				}
				else
				{
					// Only search the first element of the lists
					
					// This loop could be written in a simpler way, but I
					// am leaving it like this for the similarity to the
					// above one.
					while (true)
					{
						// Look at the first item in the current list
						if (columns[j].cols.front().erConfig == configNo)
						{
							columns[i].adjList.emplace_back(j,0);
								
							break;
						}
						
						// Not in this class, try the next one.
						j++;
					}
				}
			}
			else if (after != rev && succeeds(before, rev, result))
			{
				if (trace)
					std::cout << "yes, uses backwards form, config: " << result << std::endl;
				unsigned configNo = findER(columns[j].cols.front().numComponents, result);
				
				// We are requiring that after != rev (otherwise this check
				// would be identical to the previous one), so we only
				// need to check the first element.
				
				while (true)
				{
					// Look at the second (last) item in the current list
					if (columns[j].cols.back().erConfig == configNo)
					{
						columns[i].adjList.emplace_back(j,1);
							
						break;
					}
					
					// Not in this class, try the next one.
					j++;
				}
			}
			else if (trace) std::cout << "no" << std::endl;
			
			// Update j to be the index of the next unique physical form.
			while (j < columns.size() && columns[j].cols.front().physical == after) j++;
		}
	}
}

struct path_info
{
	// -1 for -inf
	int num_induced;
	
	// The column class that precedes this one in the optimal path.
	unsigned second_to_last;
	
	// The column number to be used from the current class
	unsigned col_no;
	
	
	path_info() : num_induced(-1), second_to_last(0), col_no(0) {}
	
	path_info(int n_i, unsigned s_t_l, unsigned c_n) :
		num_induced(n_i), second_to_last(s_t_l), col_no(c_n) {}
};

// All path info for one length and starting column.
// .info[s] gives the information about the path from
// the predetermined start column to s, of a predetermined length.
struct path_info_vector
{
	std::vector<path_info> info;
	
	path_info_vector() : info(columns.size()) {}
};

// .lengths[len].info[e] gives
// the info block for the densest path from
// the given start column to e of length len.
struct path_info_matrix
{
	std::vector<path_info_vector> lengths;
	
	path_info_matrix() : lengths(columns.size() + 2) {}
};

rectangle extractRectangle(path_info_matrix paths_info, const unsigned len, unsigned end)
{
	rectangle r;
	
	unsigned currentColumn = end;
	for (unsigned length = len; length > 0; length--)
	{
		r.append(currentColumn, paths_info.lengths[length].info[currentColumn].col_no);
		
		currentColumn = paths_info.lengths[length].info[currentColumn].second_to_last;
	}
	
	return r;
}

// Recursive helper function to enumerateTiles.
void enumerateTilesRecursive(tile& t)
{
	if (t.columnPath.size() >= n) return;

	unsigned start = t.columnPath.front().first;
	unsigned currentEnd = t.columnPath.back().first;
	
	for (auto& adj : columns[currentEnd].adjList)
	{
		// Perform a recursive search
		unsigned numVert = columns[adj.first].cols.front().numVertices;
		
		t.columnPath.push_back(adj);
		t.numVertices += numVert;
		
		enumerateTilesRecursive(t);
		
		t.columnPath.pop_back();
		t.numVertices -= numVert;
	}
}

void enumerateTiles()
{
	// Using each column as a starting point
	for (unsigned i = 0; i < columns.size(); i++)
	{
		tile t(i);
		enumerateTilesRecursive(t);
	}
}

int main(int argn, char** args)
{
	parseArgs(argn, args);
	
	produceConfigs();
	
	produceColumns();
	
	sortGreedy();
	
	fillInAdjLists();
	
	if (trace)
	{
		std::cout << "Final adjacency lists (numerical form):" << std::endl;
		
		for (unsigned i = 0; i < columns.size(); i++)
		{
			std::cout << i << ':';
			
			for (auto& pair : columns[i].adjList)
				std::cout << " (" << pair.first << ',' << pair.second << ')';
			
			std::cout << std::endl;
		}
	}
	
	enumerateTiles();
}
