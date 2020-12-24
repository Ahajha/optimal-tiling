#ifndef VARIADIC_ARRAY_HPP
#define VARIADIC_ARRAY_HPP

/*
The purpose of this template is to convert a list of template
parameters into a constexpr std::array of those parameters.
*/

#include <array>

// Base case, for zero-length template lists.
template<class T, T... ts>
struct variadic_array : public std::array<T, 0> {};

// Recursive case.
template<class T, T t1, T ... ts>
struct variadic_array<T,t1,ts...> : public std::array<T, sizeof...(ts) + 1>
{
	constexpr variadic_array()
	{
		// Store a copy of a one-smaller sized array.
		constexpr variadic_array<T,ts...> sa;
		
		// Copy the smaller array in and add the first element.
		std::copy(sa.begin(), sa.end(), this->begin() + 1);
		this->front() = t1;
	}
};

#endif
