#ifndef SLICE_HPP
#define SLICE_HPP

#include <array>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "permutation.hpp"

// TODO: Implement tracing, likely with some compiled-in macro

// Contains various type aliases, structures, constants, and
// static variables common to all sizes of slices.
namespace slice_defs
{
	// This likely will not need to be expanded, if it does
	// performance in other areas will probably be more of a concern.
	// Reserving 2 values, this gives a max of 254 components.
	// This is also used for the size of equivalence relation elements.
	using compNumType = uint8_t;
	
	// Size of equivalence relation IDs. 16 bits is only enough for all 8 element
	// equivalence relations, and 32 should be plenty.
	using er_id_type = uint32_t;
	
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
};

template<std::unsigned_integral T, T ... dims>
struct slice_base
{
	using pset = permutationSet<T,dims...>;
	struct compNumArray : std::array<slice_defs::compNumType, pset::numVertices>
	{
		// Comparisons only consider the physical form, not arrangements
		// of the components within the array.
		constexpr std::strong_ordering operator<=>(const compNumArray& other) const;
		
		constexpr bool operator==(const compNumArray& other) const
			{ return (*this <=> other) == std::strong_ordering::equal; }
	};
	
	unsigned numVerts;
	slice_defs::compNumType numComps;
	
	// Maps ER IDs to vertex IDs.
	std::unordered_map<unsigned,unsigned> er_map;
	
	slice_base(bool v) : numVerts(v), numComps(v)
		{ static_assert(sizeof...(dims) == 0); }
	slice_base(unsigned nv, slice_defs::compNumType nc) : numVerts(nv), numComps(nc)
		{ static_assert(sizeof...(dims) > 0); }
	
	constexpr static void permute(unsigned permID, const compNumArray& src,
		compNumArray& result);
	
	static bool succeeds(const compNumArray& afterCN, unsigned afterNumComp,
		const compNumArray& beforeCN, unsigned beforeERID, unsigned& result);
	
	void constructForm(const std::vector<unsigned>& path, compNumArray& out);
};

template<std::unsigned_integral T, T ... dims>
struct unpruned_slice : slice_base<T,dims...>
{
	typename slice_base<T,dims...>::compNumArray form;
	
	constexpr unpruned_slice(bool v) : slice_base<T,dims...>(v),
		form({v ? static_cast<slice_defs::compNumType>(0)
		        : slice_defs::COMPLETELY_EMPTY}) {}
	
	unpruned_slice(const std::vector<unsigned>& path, unsigned nv)
		: slice_base<T,dims...>(nv,0)
	{
		slice_base<T,dims...>::constructForm(path,form);
	}
};

template<std::unsigned_integral T, T ... dims>
struct pruned_slice : slice_base<T,dims...>
{
	// Inner vector contains configs with the same physical form, but different
	// equivalence relations. Outer vector contains symmetries with different
	// physical forms. Possible todo: use unordered_set
	std::vector<typename slice_base<T,dims...>::compNumArray> forms;
	
	template<std::unsigned_integral t, t ... ds>
	friend std::ostream& operator<<(std::ostream&,
		const pruned_slice<t,ds...>&);
	
	pruned_slice(bool v) : slice_base<T,dims...>(v),
		forms({{v ? static_cast<slice_defs::compNumType>(0)
		        : slice_defs::COMPLETELY_EMPTY}}) {}
	
	// Returns true iff this slice should be pruned.
	bool fillOrPrune();
	
	pruned_slice(const std::vector<unsigned>& path, unsigned nv)
		: slice_base<T,dims...>(nv,0)
	{
		slice_base<T,dims...>::constructForm(path,forms.emplace_back());
	}
};

template<bool prune, std::unsigned_integral T, T ... dims>
struct slice_graph
{
	using slice_t = std::conditional<prune,
		pruned_slice<T,dims...>, unpruned_slice<T,dims...>>::type;
	
	static inline std::vector<slice_defs::vertex> graph{};
	static inline std::vector<slice_t> slices{};
	
	static slice_t& lookup(unsigned vID)
	{
		return slices[graph[vID].sliceNum];
	}
	
	// Fills graph and slices.
	static void enumerate();
	
	private:
	
	static void enumerateRecursive(std::vector<unsigned>& path, unsigned& nv);
	
	static void fillVertex(unsigned vID);
	
	static void addVertex(unsigned sliceID, unsigned erID)
	{
		slices[sliceID].er_map[erID] = graph.size();
		graph.emplace_back(sliceID,erID);
	}
	
	static unsigned getVertex(unsigned sliceID, unsigned erID);
};

// Due to a long standing GCC bug, this is a struct rather than
// part of the slice_defs namespace.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59498
template<std::unsigned_integral T, T d1, T ... rest>
struct slice_alias
{
	// Subgraphs are never pruned
	using sub_graph = slice_graph<false,T,rest...>;
	
	using sub_slice = slice_base<T,rest...>;
	
	constexpr static T primary_dim = d1;
};

#include "slice.tpp"

#endif
