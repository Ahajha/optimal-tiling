#ifndef FRACTION_HPP
#define FRACTION_HPP

#include <iostream>
#include <compare>

// A positive fraction
struct fraction
{
	// den == 0 implies num == 0. Not the most intuitive, but makes some other
	// things here simpler.
	unsigned num, den;
	
	std::strong_ordering operator<=>(const fraction& other) const;
	
	// Defaults to 0, otherwise takes a specified numerator and denominator.
	fraction();
	fraction(unsigned, unsigned);
};

std::ostream& operator<<(std::ostream&, const fraction&);

#endif
