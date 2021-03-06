#ifndef PERMUTATION_HPP
#define PERMUTATION_HPP

/*
A class to store permutations of a hyper-rectangular-prismic lattice graph.
The first size given should be the "highest" dimension.
*/

#include <array>

template<std::unsigned_integral T, T ...>
struct permutationSet
{
	constexpr static T numVertices = 1;
	
	using permutation = std::array<T,1>;
	
	constexpr static std::array<permutation,1> perms = {{0}};
};

template<std::unsigned_integral T, T d1, T ... rest>
struct permutationSet<T,d1,rest...>
{
	constexpr static T numVertices = (d1 * ... * rest);
	
	using permutation = std::array<T,numVertices>;
	
	private:
	
	// Count number of dimensions equal to d1. (d1 == d1, so start at 1)
	constexpr static unsigned similarDimensions = (1 + ... + (d1 == rest));
	
	constexpr static std::array<permutation,
		permutationSet<T, rest...>::perms.size() * 2 * similarDimensions> makePerms();

	public:
	
	constexpr static auto perms = makePerms();
};

#include "permutation.tpp"

#endif
