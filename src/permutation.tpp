#include "permutation.hpp"

template<std::unsigned_integral T, T d1, T ... rest>
constexpr auto permutationSet<T, d1, rest...>::makePerms() -> std::array<permutation,
		permutationSet<T,rest...>::perms.size() * 2 * similarDimensions>
{
	constexpr std::array<T,1 + sizeof...(rest)> dims {{d1,rest...}};
	
	// Array of sizes of previous dimensions, effectively a running product
	// of the dimension sizes up to a point. The running product goes from
	// back to front, with the number at index 0 representing the size
	// of one dimension lower than the maximum given.
	std::array<T, dims.size()> dimSizes;
	dimSizes.back() = 1;
	for (unsigned i = 1; i < dims.size(); ++i)
	{
		dimSizes[dims.size() - i - 1] =
			dimSizes[dims.size() - i] * dims[dims.size() - i];
	}
	
	constexpr auto& subPerms = permutationSet<T,rest...>::perms;
	
	std::array<permutation, permutationSet<T,rest...>::perms.size()
		* 2 * similarDimensions> perms;
	
	T index = 0;
	for (const auto& subPerm : subPerms)
	{
		auto helper = [&](auto& ref)
		{
			// Skip over the primary dimension, since there is no point
			// swapping with itself (and we have constructed the 'reference'
			// permutation) and look for any other dimensions that
			// shares the same size, swap that dimension with the primary one.
			for (T i = 1; i < dims.size(); ++i)
			{
				if (dims[i] == d1)
				{
					// For each element in the reference permutation,
					// extract the last dimension and dimension i and swap them.
					for (T j = 0; j < numVertices; ++j)
					{
						// Copy over value from the reference permutation
						auto& val = perms[index][j] = ref[j];
						
						// Dimension 'primary' and 'i'
						T dp =  val / dimSizes.front();
						T di = (val / dimSizes[i]) % dims[i];
						
						// 'erase' the dimensions, then re-add them, but swapped.
						val -= (dp * dimSizes.front() + di * dimSizes[i]);
						val += (di * dimSizes.front() + dp * dimSizes[i]);
					}
					
					++index;
				}
			}
		};
		
		// Construct two 'reference' permutations, with one having the primary
		// dimension reversed.
		auto& ref = perms[index++];
		auto& reverseRef = perms[index++];
		
		// Construct an initial form that just adds the next dimension
		for (T i = 0; i < d1; ++i)
		{
			for (T j = 0; j < dimSizes.front(); ++j)
			{
				ref[i * dimSizes.front() + j] =
					i * dimSizes.front() + subPerm[j];
				
				reverseRef[i * dimSizes.front() + j] =
					(d1 - i - 1) * dimSizes.front() + subPerm[j];
			}
		}
		
		helper(ref);
		helper(reverseRef);
	}
	
	return perms;
}
