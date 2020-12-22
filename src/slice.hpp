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
namespace slice_defs
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
	
	static inline er_storage er_store{};
};

template<auto dims>
	requires unsigned_range<decltype(dims)>
struct slice_base
{
	using pset = permutationSet<dims>;
	using compNumArray = std::array<slice_defs::compNumType, pset::numVertices>;
	
	unsigned numVerts;
	slice_defs::compNumType numComps;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	protected:
	
	static void permute(unsigned permID, const compNumArray& src,
		compNumArray& result);
	
	static std::strong_ordering compareSymmetries
		(const compNumArray& sym1, const compNumArray& sym2);
	
	static bool succeeds(const compNumArray& afterCN, unsigned afterNumComp,
		const compNumArray& beforeCN, unsigned beforeERID, unsigned& result);
};

template<auto dims>
	requires unsigned_range<decltype(dims)>
struct unpruned_slice : public slice_base<dims>
{
	typename slice_base<dims>::compNumArray form;
	
	template<auto d>
		requires unsigned_range<decltype(d)>
	friend std::ostream& operator<<(std::ostream&, const unpruned_slice<d>&);
	
	// TODO: specialization for dims.size() == 1 or 0
	unpruned_slice(const std::vector<unsigned>& path, unsigned nv);
};

template<auto dims>
	requires unsigned_range<decltype(dims)>
struct pruned_slice : public slice_base<dims>
{
	// Inner vector contains configs with the same physical form, but different
	// equivalence relations. Outer vector contains symmetries with different
	// physical forms. TODO: use unordered_set?
	std::vector<std::vector<typename slice_base<dims>::compNumArray>> forms;
	
	template<auto d>
		requires unsigned_range<decltype(d)>
	friend std::ostream& operator<<(std::ostream&, const pruned_slice<d>&);
	
	// TODO: specialization for dims.size() == 1 or 0
	pruned_slice(const std::vector<unsigned>& path, unsigned nv);
};

template<auto dims, bool prune = true>
	requires unsigned_range<decltype(dims)>
struct slice_graph
{
	using slice_t = std::conditional<prune,
		pruned_slice<dims>, unpruned_slice<dims>>;
	
	static inline std::vector<slice_defs::vertex> graph{};
	static inline std::vector<slice_t> slices{};
	
	// Fills graph and slices.
	static void enumerate();
	
	private:
	
	static void enumerateRecursive(std::vector<unsigned>& path, unsigned nv);
	
	static void fillVertex(unsigned vID);
	
	static slice_t& lookup(unsigned vID);
	
	static void addVertex(unsigned sliceID, unsigned erID);
};

#include "slice.tpp"

#endif
