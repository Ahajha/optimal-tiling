#ifndef FRACTION_HPP
#define FRACTION_HPP

#include <iostream>
#include <compare>

// A positive fraction
struct fraction
{
	// den == 0 implies num == 0. Not the most intuitive, but makes some other
	// things here simpler. In the future, will default to 0/1 instead.
	unsigned num, den;
	
	std::strong_ordering operator<=>(const fraction& other) const;
	
	// Defaults to 0/0, can specify either just numerator or both.
	fraction(unsigned n = 0, unsigned d = 0) : num(n), den(d) {}
};

std::ostream& operator<<(std::ostream&, const fraction&);

#endif
