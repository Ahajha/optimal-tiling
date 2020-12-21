#ifndef SLICE_HPP
#define SLICE_HPP

#include <array>
#include <vector>
#include <iostream>
#include <unordered_set>
#include "permutation.hpp"
#include "equivRelation.hpp"

// TODO: Implement tracing, likely with some compiled-in macro

// Contains various type aliases, structures, constants, and
// static variables common to all sizes of slices.
struct slice_base
{
	// This likely will not need to be expanded, if it does
	// performance in other areas will probably be more of a concern.
	// Reserving 2 values, this gives a max of 254 components.
	using compNumType = uint8_t;
	
	// Reserve the largest two values
	constexpr static compNumType EMPTY =
		std::numeric_limits<compNumType>::max();
	constexpr static compNumType COMPLETELY_EMPTY = EMPTY - 1;
	
	struct vertex
	{
		std::vector<unsigned> adjList;
		unsigned sliceNum, erID;
		
		vertex(unsigned sn, unsigned e) : sliceNum(sn), erID(e) {}
	};
};

namespace slice_er_storage
{
	static inline er_storage store{};
};

// TODO: implement changes when prune == true
template<auto dims, bool prune = false>
	requires unsigned_range<decltype(dims)>
struct slice : public slice_base
{
	// =========================== Public interface ============================
	
	using pset = permutationSet<dims>;
	using compNumArray = std::array<compNumType, pset::numVertices>;
	
	// Inner vector contains configs with the same physical form, but different
	// equivalence relations. Outer vector contains symmetries with different
	// physical forms. TODO: use unordered_set?
	std::vector<std::vector<compNumArray>> forms;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	unsigned numVerts;
	compNumType numComps; 
	
	static inline std::vector<vertex> graph{};
	static inline std::vector<slice> slices{};
	
	// Fills graph and slices.
	static void enumerate();
	
	template<auto d, bool p>
		requires unsigned_range<decltype(d)>
	friend std::ostream& operator<<(std::ostream&, const slice<d,p>&);
	
	static slice& lookup(unsigned vID);
	
	// ================================ Private ================================
	
	private:
	
	// TODO: specialization for dims.size() == 1
	slice(const std::vector<unsigned>& path, unsigned nv);
	
	static void permute(unsigned permID, const compNumArray& src,
		compNumArray& result);
	
	static void enumerateRecursive();
	
	static void fillVertex(unsigned vID);
	
	static void addVertex(unsigned sliceID, unsigned erID);
	
	static std::strong_ordering compareSymmetries
		(const compNumArray& sym1, const compNumArray& sym2);
	
	static bool succeeds(const compNumArray& afterCN, unsigned afterNumComp,
		const compNumArray& beforeCN, unsigned beforeERID, unsigned& result);
};

#include "slice.tpp"

#endif
