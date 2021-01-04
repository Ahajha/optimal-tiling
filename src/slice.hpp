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
	
	constexpr static bool empty(compNumType v)
		{ return v >= COMPLETELY_EMPTY; }
	
	struct vertex
	{
		std::vector<unsigned> adjList;
		unsigned sliceNum, erID;
		
		vertex(unsigned sn, unsigned e) : sliceNum(sn), erID(e) {}
	};
	
	static inline er_storage er_store{};
};

// For 0 dimensions
template<std::unsigned_integral T, T ... dims>
struct slice_base
{
	using pset = permutationSet<T,dims...>;
	using compNumArray = std::array<slice_defs::compNumType, pset::numVertices>;
	
	unsigned numVerts;
	slice_defs::compNumType numComps;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	slice_base(bool v);
};

// For 1+ dimensions
template<std::unsigned_integral T, T d1, T ... rest>
struct slice_base<T,d1,rest...>
{
	using pset = permutationSet<T,d1,rest...>;
	using compNumArray = std::array<slice_defs::compNumType, pset::numVertices>;
	
	unsigned numVerts;
	slice_defs::compNumType numComps;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	slice_base(unsigned nv, slice_defs::compNumType nc);
	
	static void permute(unsigned permID, const compNumArray& src,
		compNumArray& result);
	
	static std::strong_ordering compareSymmetries
		(const compNumArray& sym1, const compNumArray& sym2);
	
	static bool succeeds(const compNumArray& afterCN, unsigned afterNumComp,
		const compNumArray& beforeCN, unsigned beforeERID, unsigned& result);
	
	void constructForm(const std::vector<unsigned>& path, compNumArray& out);
};

// For 0 dimensions
template<std::unsigned_integral T, T ... dims>
struct unpruned_slice : public slice_base<T,dims...>
{
	typename slice_base<T,dims...>::compNumArray form;
	
	template<std::unsigned_integral t, t ... ds>
	friend std::ostream& operator<<(std::ostream&,
		const unpruned_slice<t,ds...>&);
	
	unpruned_slice(bool v);
};

// For 1+ dimensions (minor todo: specialization for 1 dimension)
template<std::unsigned_integral T, T d1, T ... rest>
struct unpruned_slice<T,d1,rest...> : public slice_base<T,d1,rest...>
{
	typename slice_base<T,d1,rest...>::compNumArray form;
	
	template<std::unsigned_integral t, t d, t ... ds>
	friend std::ostream& operator<<(std::ostream&,
		const unpruned_slice<t,d,ds...>&);
	
	unpruned_slice(const std::vector<unsigned>& path, unsigned nv);
};

// For 0 dimensions
template<std::unsigned_integral T, T ... dims>
struct pruned_slice : public slice_base<T,dims...>
{
	// Inner vector contains configs with the same physical form, but different
	// equivalence relations. Outer vector contains symmetries with different
	// physical forms. Possible todo: use unordered_set
	std::vector<std::vector<
		typename slice_base<T,dims...>::compNumArray
	>> forms;
	
	template<std::unsigned_integral t, t ... ds>
	friend std::ostream& operator<<(std::ostream&,
		const pruned_slice<t,ds...>&);
	
	pruned_slice(bool v);
};

// For 1+ dimensions (minor todo: specialization for 1 dimension)
template<std::unsigned_integral T, T d1, T ... rest>
struct pruned_slice<T,d1,rest...> : public slice_base<T,d1,rest...>
{
	std::vector<std::vector<
		typename slice_base<T,d1,rest...>::compNumArray
	>> forms;
	
	template<std::unsigned_integral t, t d, t ... ds>
	friend std::ostream& operator<<(std::ostream&,
		const pruned_slice<t,d,ds...>&);
	
	pruned_slice(const std::vector<unsigned>& path, unsigned nv);
};

template<bool prune, std::unsigned_integral T, T ... dims>
struct slice_graph
{
	using slice_t = std::conditional<prune,
		pruned_slice<T,dims...>, unpruned_slice<T,dims...>>::type;
	
	static inline std::vector<slice_defs::vertex> graph{};
	static inline std::vector<slice_t> slices{};
	
	static slice_t& lookup(unsigned vID);
	
	// Fills graph and slices.
	static void enumerate();
	
	private:
	
	static void addVertex(unsigned sliceID, unsigned erID);
};

template<bool prune, std::unsigned_integral T, T d1, T ... rest>
struct slice_graph<prune,T,d1,rest...>
{
	using slice_t = std::conditional<prune,
		pruned_slice<T,d1,rest...>, unpruned_slice<T,d1,rest...>>::type;
	
	static inline std::vector<slice_defs::vertex> graph{};
	static inline std::vector<slice_t> slices{};
	
	static slice_t& lookup(unsigned vID);
	
	// Fills graph and slices.
	static void enumerate();
	
	private:
	
	static void enumerateRecursive(std::vector<unsigned>& path, unsigned& nv);
	
	static void fillVertex(unsigned vID);
	
	static void addVertex(unsigned sliceID, unsigned erID);
};

namespace slice_defs
{
	template<std::unsigned_integral T, T, T ... rest>
	using sub_graph = slice_graph<false,T,rest...>;
	
	template<std::unsigned_integral T, T, T ... rest>
	using sub_slice = slice_base<T,rest...>;
};

#include "slice.tpp"

#endif
