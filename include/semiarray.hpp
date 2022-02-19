#pragma once

/*
A semi-array is a constexpr partially filled std::array.
*/

#include <array>

template<class T, std::size_t N>
class semiarray : public std::array<T,N>
{
	std::size_t _size;
	
	public:
	
	[[nodiscard]] constexpr semiarray() : _size(0) {}
	
	constexpr void push_back(T t) { this->operator[](_size++) = t; }
	
	[[nodiscard]] constexpr auto     end()       { return std::array<T,N>::begin()  + _size; }
	[[nodiscard]] constexpr auto     end() const { return std::array<T,N>::begin()  + _size; }
	[[nodiscard]] constexpr auto    cend() const { return std::array<T,N>::cbegin() + _size; }
	[[nodiscard]] constexpr auto  rbegin()       { return std::array<T,N>::rend()   - _size; }
	[[nodiscard]] constexpr auto  rbegin() const { return std::array<T,N>::rend()   - _size; }
	[[nodiscard]] constexpr auto crbegin() const { return std::array<T,N>::crend()  - _size; }
	
	[[nodiscard]] constexpr T back() const { return this->operator[](_size - 1); }
	
	[[nodiscard]] constexpr bool empty() const { return _size == 0; }
	
	[[nodiscard]] constexpr std::size_t size() const { return _size; }
};
