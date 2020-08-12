#include "equivRelation.hpp"

equivRelation::equivRelation() : elements(0) {}

equivRelation::equivRelation(unsigned size) : elements(size)
{
	for (unsigned i = 0; i < size; i++)
		elements[i] = element(i);
}

void equivRelation::merge(unsigned x, unsigned y)
{
	unsigned leadX = leader(x), leadY = leader(y);
	
	if (leadX != leadY)
	{
		if (elements[leadX].numConstituents < elements[leadY].numConstituents)
		{
			elements[leadY].numConstituents += elements[leadX].numConstituents;
			elements[leadX].numConstituents = 0;
			elements[leadX].boss = leadY;
		}
		else
		{
			elements[leadX].numConstituents += elements[leadY].numConstituents;
			elements[leadY].numConstituents = 0;
			elements[leadY].boss = leadX;
		}
	}
}

bool equivRelation::equivalent(unsigned x, unsigned y) const
{
	return leader(x) == leader(y);
}

void equivRelation::addElement()
{
	elements.emplace_back(elements.size());
}

bool equivRelation::operator==(const equivRelation& other) const
{
	if (elements.size() != other.elements.size()) return false;
	
	auto cgl1 = canonicalGroupLabeling();
	auto cgl2 = other.canonicalGroupLabeling();
	
	return cgl1 == cgl2;
}

std::strong_ordering equivRelation::operator<=>(const equivRelation& other) const
{
	if (elements.size() != other.elements.size())
		return elements.size() <=> other.elements.size();
	
	auto cgl1 = canonicalGroupLabeling();
	auto cgl2 = other.canonicalGroupLabeling();
	
	// I would implement this as just "return cgl1 <=> cgl2;",
	// but that does not work, for some unknown reason.
	for (unsigned i = 0; i < elements.size(); i++)
	{
		if (cgl1[i] != cgl2[i])
			return cgl1[i] <=> cgl2[i];
	}
	return std::strong_ordering::equal;
}

std::ostream& operator<<(std::ostream& stream, const equivRelation& R)
{
	auto cgl = R.canonicalGroupLabeling();
	
	for (unsigned label : cgl) stream << label << ' ';
	
	return stream;
}

unsigned equivRelation::leader(unsigned x) const
{
	if (elements[x].boss == x) return x;
	
	return (elements[x].boss = leader(elements[x].boss));
}

std::vector<equivRelation> equivRelation::enumerate(unsigned n)
{
	std::vector<equivRelation> result;
	if (n == 1)
	{
		// Only one of size 1
		result.emplace_back(1);
	}
	else
	{
		std::vector<equivRelation> smallerERs = enumerate(n-1);

		for (auto& R : smallerERs)
		{
			// For each group in R, there will be a new ER
			// with the last member merged with that group.
			for (unsigned i = 0; i < n - 1; i++)
			{
				// Since we are looking for the unique groups, just look for the leaders.
				if (R.elements[i].boss == i)
				{
					// Add a copy of R
					result.emplace_back(R);
		
					// Add an element to back of the copy, make it
					// equivalent to i.
					result.back().addElement();
					result.back().merge(i,n - 1);
				}
			}
			
			// The last option is to have on element not equivalent to anything
			result.emplace_back(R);

			result.back().addElement();
		}
	}
	return result;
}

equivRelation equivRelation::reverse() const
{
	unsigned n = elements.size(); // for brevity
	equivRelation result(n);
	
	for (unsigned i = 0; i < n; i++)
	{
		result.elements[n - i - 1].numConstituents = elements[i].numConstituents;
		result.elements[n - i - 1].boss = n - leader(i) - 1;
	}
	
	return result;
}

equivRelation equivRelation::append(const equivRelation& other) const
{
	// Start with a copy, then add enough elements to add the ones from other
	equivRelation result(*this);
	result.elements.resize(elements.size() + other.elements.size());
	
	// For each element in result, numConstituents should be the same as
	// the corresponding one in other, and the boss should be the
	// same plus the number of elements in this.
	for (unsigned i = 0; i < other.elements.size(); i++)
	{
		result.elements[elements.size() + i].numConstituents =
			other.elements[i].numConstituents;
		result.elements[elements.size() + i].boss =
			other.elements[i].boss + elements.size();
	}
	
	return result;
}

void equivRelation::operator+=(const equivRelation& other)
{
	unsigned oldSize = elements.size();

	elements.resize(oldSize + other.elements.size());
	
	// For each element in result, numConstituents should be the same as
	// the corresponding one in other, and the boss should be the
	// same plus the number of elements in this.
	for (unsigned i = 0; i < other.elements.size(); i++)
	{
		elements[oldSize + i].numConstituents =
			other.elements[i].numConstituents;
		elements[oldSize + i].boss =
			other.elements[i].boss + oldSize;
	}
}

equivRelation equivRelation::shave(unsigned n) const
{
	auto cgl = canonicalGroupLabeling();
	
	std::vector<unsigned> leaders;
	
	equivRelation result(elements.size() - n);
	
	for (unsigned i = 0; i < result.elements.size(); i++)
	{
		// Clear the number of constituents
		result.elements[i].numConstituents = 0;
	}
	
	for (unsigned i = 0; i < result.elements.size(); i++)
	{
		unsigned groupNum = cgl[i];
	
		// If a new group is seen, mark i as the leader of
		// this new group.
		if (groupNum == leaders.size()) leaders.push_back(i);
		
		// Look up which element is the leader of i.
		result.elements[i].boss = leaders[groupNum];
		
		// Increment the number of constituents for the leader of i
		result.elements[leaders[groupNum]].numConstituents++;
	}
	
	return result;
}

unsigned equivRelation::size() const
{
	return elements.size();
}

unsigned equivRelation::numComponents() const
{
	unsigned result = 0;
	
	for (unsigned i = 0; i < elements.size(); i++)
	{
		if (elements[i].boss == i) result++;
	}
	
	return result;
}

std::vector<unsigned> equivRelation::canonicalGroupLabeling() const
{
	std::vector<unsigned> result(elements.size());
	std::fill(result.begin(), result.end(), (unsigned)(-1));
	
	unsigned group_num = 0;
	for (unsigned i = 0; i < elements.size(); i++)
	{
		// If an element has already been marked, then it is
		// a leader, we can ignore this case and just let it
		// get overwritten again.
	
		unsigned lead = leader(i);
		if (result[lead] == (unsigned)(-1))
			result[lead] = group_num++;
		
		result[i] = result[lead];
	}
	
	return result;
}

std::size_t er_hash::operator()(const equivRelation& er)
{
	// This implementation is based off djb2, found at
	// http://www.cse.yorku.ca/~oz/hash.html
	
	std::size_t hash = 5381;
	
	const auto cgl = er.canonicalGroupLabeling();
	
	for (auto c : cgl)
	{
		// Adding 1 here, just in case 0s mess with anything.
		hash = ((hash << 5) + hash) + c + 1; /* hash * 33 + c */
	}
	
	return hash;
}
