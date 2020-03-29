#ifndef INDEXED_LIST_HPP
#define INDEXED_LIST_HPP

#include <cstddef>
#include <array>
#include "defs.hpp"

/*
An indexedList is an array/doubly linked list hybrid,
with the benefits of both. It requires a fixed set of
integers 0-N (excluding N) to be the only candidates
for the list, without repeats. All insertions and
removals are in constant time.
*/

template<std::size_t N>
class indexedList
{
	private:
	
	struct index;

	public:
	
	indexedList();

	// Returns true if x was removed, and false if x did not already exist here.
	// Remove and both pop methods assume there is at least one item in the list.
	bool remove(defs::vertexID);
	
	void push_front(defs::vertexID);
	void push_back (defs::vertexID);
	
	defs::vertexID  pop_front ();
	defs::vertexID  pop_back  ();
	
	bool empty() const;
	
	bool exists(defs::vertexID) const;
	
	void clear();
	
	// Removes a random item from the list and returns it, uniformly distributed.
	// Assumes there is an item to remove.
	defs::vertexID removeRandom();
	
	unsigned size() const;
	
	void print();
	
	friend class iterator;
	
	class iterator
	{
		public:
		
		iterator(std::array<index, N>& li, defs::vertexID x) : currentNode(x), list(li) {}
		
		iterator& operator++()
		{
			if (currentNode != defs::EMPTY)
				currentNode = list[currentNode].next;
			return *this;
		}
		
		iterator  operator++(int)
		{
			iterator it = *this;
			++*this;
			return it;
		}
		
		bool operator !=(const iterator& i)
		{
			return currentNode != i.currentNode;
		}
		
		defs::vertexID operator*() { return currentNode; }
		
		private:
		
		defs::vertexID currentNode;
		const std::array<index, N>& list;
	};
	
	iterator begin() { return iterator(list,head); }
	iterator end  () { return iterator(list,defs::EMPTY); }
	
	template <std::size_t X>
	friend void swap(indexedList<X>&,indexedList<X>&);
	
	private:
	
	struct index
	{
		bool inList;
		defs::vertexID next, prev;
		
		index() : inList(false), next(defs::EMPTY), prev(defs::EMPTY) {}
	};
	
	unsigned numItems;
	
	std::array<index, N> list;
	
	defs::vertexID head, tail;
};

#include "indexedList.tpp"

#endif
