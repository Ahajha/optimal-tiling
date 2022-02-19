#pragma once

#include <limits>
#include <cstdint>
#include <type_traits>

/*
Provides a type which is the minimum fastest type that can hold a
specified maximum value, as well as the value cast to that type.
*/

template<std::size_t maxVal>
struct min_fast_type
{
private:
	constexpr static bool fits_in_u8  = maxVal <= std::numeric_limits<std::uint8_t >::max();
	constexpr static bool fits_in_u16 = maxVal <= std::numeric_limits<std::uint16_t>::max();
	constexpr static bool fits_in_u32 = maxVal <= std::numeric_limits<std::uint32_t>::max();
public:
	using type = std::conditional_t<fits_in_u8,  uint_fast8_t,
	             std::conditional_t<fits_in_u16, uint_fast16_t,
	             std::conditional_t<fits_in_u32, uint_fast32_t,
	                                             uint_fast64_t>>>;
	
	constexpr static type value = static_cast<type>(maxVal);
};
