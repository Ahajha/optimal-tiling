#ifndef VARIADIC_ARRAY_HPP
#define VARIADIC_ARRAY_HPP

/*
The purpose of this template is to convert a list of template
parameters into a consteval std::array of those parameters.
*/

#include <array>

template<class T, T... ts>
struct variadic_array : public std::array<T, sizeof...(ts)> {};

template<class T, T t1, T ... ts>
struct variadic_array<T,t1,ts...> : public std::array<T, sizeof...(ts) + 1>
{
	consteval variadic_array()
	{
		constexpr variadic_array<T,ts...> sa;
		std::copy(sa.begin(), sa.end(), this->begin() + 1);
		this->front() = t1;
	}
};

#endif
