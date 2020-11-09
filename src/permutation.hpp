#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

#include <array>
#include <ranges>

template<class T>
concept unsigned_range = std::ranges::random_access_range<T>
	&& std::unsigned_integral<typename T::value_type>;

template<auto dims>
	requires unsigned_range<decltype(dims)>
struct permutationSet
{
	using value_type = typename decltype(dims)::value_type;
	
	// Removes the last element
	constexpr static auto subArray = []()
	{
		std::array<value_type, dims.size() - 1> arr;
		std::copy(dims.begin(), dims.end() - 1, arr.begin());
		return arr;
	}();
	
	constexpr static value_type primaryDim = dims[dims.size() - 1];
	
	constexpr static value_type numVertices = primaryDim
		* permutationSet<subArray>::numVertices;
	
	constexpr static value_type similarDimensions =
		std::count(dims.begin(),dims.end(),primaryDim);
	
	using permutation = std::array<value_type,numVertices>;
	
	private:
	
	constexpr static std::array<permutation,
		permutationSet<subArray>::perms.size() * 2 * similarDimensions> makePerms();

	public:
	
	constexpr static auto perms = makePerms();
};

template<auto dims>
	requires unsigned_range<decltype(dims)>
	      && (dims.empty())
struct permutationSet<dims>
{
	using value_type = typename decltype(dims)::value_type;
	
	constexpr static value_type numVertices = 1;
	
	using permutation = std::array<value_type,numVertices>;
	
	constexpr static std::array<permutation,1> perms = {{0}};
};

#include "permutation.tpp"

#endif
