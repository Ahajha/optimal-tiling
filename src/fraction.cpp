#include "fraction.hpp"
#include <iostream>
#include <compare>

std::strong_ordering fraction::operator<=>(const fraction& other) const
{
	// 0 cases
	if (den == 0) return 0 <=> other.num;
	if (other.den == 0) return num <=> 0;
	
	auto result = (num * other.den) <=> (other.num * den);
	
	// If they are equal, say the one with the smaller denominator
	// is smaller. Otherwise, compare normally.
	return (result == 0) ? other.den <=> den : result;
}

std::ostream& operator<<(std::ostream& stream, const fraction& f)
{
	return stream << f.num << "/" << f.den
		<< " = " << static_cast<double>(f.num)/f.den;
}
