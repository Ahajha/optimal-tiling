#include "fraction.hpp"
#include <iostream>
#include <compare>

fraction::fraction() : num(0), den(0) {}
fraction::fraction(unsigned n, unsigned d) : num(n), den(d) {}

std::strong_ordering fraction::operator<=>(const fraction& other) const
{
	// 0 cases
	if (den == 0) return 0 <=> other.num;
	if (other.den == 0) return num <=> 0;

	return (num * other.den) <=> (other.num * den);
}

std::ostream& operator<<(std::ostream& stream, const fraction& f)
{
	stream << f.num << "/" << f.den << " = " << ((double)f.num/f.den);
	
	return stream;
}
