#ifndef FRACTION_HPP
#define FRACTION_HPP

#include <iostream>

// A positive fraction
struct fraction
{
	// Defaults to 0/1
	unsigned num, den;
	
	// Defaults to 0/0, can specify either just numerator or both.
	fraction(unsigned n = 0, unsigned d = 1) : num(n), den(d) {}
	
	auto operator<=>(const fraction& other) const
	{
		auto result = (num * other.den) <=> (other.num * den);
		
		// If they are equal, say the one with the smaller denominator
		// is smaller. Otherwise, compare normally.
		return (result == 0) ? other.den <=> den : result;
	}
	
	friend std::ostream& operator<<(std::ostream& stream, const fraction& f)
	{
		return stream << f.num << '/' << f.den
			<< " = " << static_cast<double>(f.num)/f.den;
	}
};

#endif
