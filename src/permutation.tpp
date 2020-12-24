#include "permutation.hpp"

template<std::unsigned_integral T, T d1, T ... rest>
constexpr auto permutationSet<T, d1,rest...>::makePerms() -> std::array<permutation,
		permutationSet<T,rest...>::perms.size() * 2 * similarDimensions>
{
	// Array of sizes of previous dimensions, effectively a running product
	// of the dimension sizes up to a point.
	std::array<T, dims.size()> dimSizes;
	dimSizes[0] = 1;
	for (unsigned i = 1; i < dims.size(); ++i)
	{
		dimSizes[i] = dimSizes[i - 1] * dims[i - 1];
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
			for (T i = 0; i < dims.size() - 1; ++i)
			{
				if (dims[i] == d1)
				{
					// For each element in the reference permutation,
					// extract the last dimension and dimension i and swap them.
					for (T j = 0; j < numVertices; ++j)
					{
						auto& val = perms[index][j] = ref[j];
						
						// Dimension 'primary' and 'i'
						T dp =  val / dimSizes[dims.size() - 1];
						T di = (val / dimSizes[i]) % dims[i];
						
						// 'erase' the dimensions
						val -= dp * dimSizes[dims.size() - 1];
						val -= di * dimSizes[i];
						
						// 'add' the new dimensions, but swapped
						val += dp * dimSizes[i];
						val += di * dimSizes[dims.size() - 1];
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
			for (T j = 0; j < dimSizes[dims.size() - 1]; ++j)
			{
				ref[i * dimSizes[dims.size() - 1] + j]
					= i * dimSizes[dims.size() - 1] + subPerm[j];
				
				reverseRef[i * dimSizes[dims.size() - 1] + j] =
					(d1 - i - 1) * dimSizes[dims.size() - 1] + subPerm[j];
			}
		}
		
		helper(ref);
		helper(reverseRef);
	}
	
	return perms;
}
