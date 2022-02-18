#ifndef MIN_FAST_TYPE_HPP
#define MIN_FAST_TYPE_HPP

#include <cstdint>
#include <type_traits>

/*
Provides a type which is the minimum fastest
type that can hold a specified maximum value,
as well as the value cast to that type.
*/

template<uintmax_t maxVal>
struct minFastType
{
	using type = std::conditional_t<maxVal <= UINT8_MAX,  uint_fast8_t,
	             std::conditional_t<maxVal <= UINT16_MAX, uint_fast16_t,
	             std::conditional_t<maxVal <= UINT32_MAX, uint_fast32_t,
	                                                      uint_fast64_t>>>;
	
	constexpr static type value = static_cast<type>(maxVal);
};

#endif
