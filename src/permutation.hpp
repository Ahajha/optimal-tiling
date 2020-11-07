#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

#include <array>
#include <ranges>

template<class T>
concept unsigned_range = std::ranges::sized_range<T>
	&& std::unsigned_integral<typename T::value_type>;

template<auto dims>
	requires unsigned_range<decltype(dims)>
struct permutationSet
{
	using value_type = typename decltype(dims)::value_type;
	
	constexpr static auto subArray = []()
	{
		std::array<value_type, dims.size() - 1> arr;
		std::copy(std::next(dims.begin()),dims.end(),arr.begin());
		return arr;
	}();
	
	constexpr static value_type numVertices = *dims.begin()
		* permutationSet<subArray>::numVertices;
	
	constexpr static value_type similarDimensions =
		std::count(dims.begin(),dims.end(),*dims.begin());
	
	private:
	
	constexpr static std::array<value_type,
		permutationSet<subArray>::numVertices * 2 * similarDimensions> makePerms();

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
	
	constexpr static std::array<value_type,0> perms = {{0}};
};

#endif
