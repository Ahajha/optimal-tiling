#ifndef EQUIV_HPP
#define EQUIV_HPP

#include <vector>
#include <iostream>
#include <compare>

class equivRelation
{
	public:
	
	equivRelation();
	
	equivRelation(unsigned);
	
	// Merge two elements
	void merge(unsigned, unsigned);
	
	// Checks if two elements are equivalent
	bool equivalent(unsigned, unsigned) const;
	
	// Appends an element to the back of this ER, not equivalent
	// to any other element, and in its own new group.
	void addElement();
	
	bool operator==(const equivRelation&) const;
	std::strong_ordering operator<=>(const equivRelation&) const;
	
	// Gets all of the different possible ERs of a given size.
	static std::vector<equivRelation> enumerate(unsigned);
	
	// Returns the reverse of this relation. The reversal satisfies
	// the property that this.equiv(x,y) iff this.reverse().equiv(n-x,n-y).
	equivRelation reverse() const;
	
	// Returns the result of appending another ER to this one. Both sets
	// of numbers will be mutually non-equivalent.
	equivRelation append(const equivRelation&) const;
	
	// Similar to append, but modifies this instead.
	void operator+=(const equivRelation&);
	
	// Returns the result of removing a given number of items from the
	// end of this ER.
	equivRelation shave(unsigned) const;

	// Returns the canonical group labeling of this.
	std::vector<unsigned> canonicalGroupLabeling() const;

	// Prints the canonical group labeling of this ER to an output stream.
	friend std::ostream& operator<<(std::ostream&, const equivRelation&);
	
	// Returns the number of elements in this ER.
	unsigned size() const;
	
	// Returns the number of non-equivalent components of this ER.
	unsigned numComponents() const;
	
	private:
	
	unsigned leader(unsigned) const;
	
	struct element
	{
		unsigned numConstituents;
		
		// Mutable, since the leader function should appear
		// const, but technically isn't.
		mutable unsigned boss;
		
		element() {}
	
		element(unsigned index) :
			numConstituents(1), boss(index) {}
	};
	
	std::vector<element> elements;
};

#endif
