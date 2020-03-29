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
	bool remove(vertexID x);
	
	void push_front(vertexID x);
	void push_back (vertexID x);
	
	vertexID  pop_front ();
	vertexID  pop_back  ();
	
	bool empty() const;
	
	bool exists(vertexID x) const;
	
	void clear();
	
	// Removes a random item from the list and returns it, uniformly distributed.
	// Assumes there is an item to remove.
	vertexID removeRandom();
	
	unsigned size() const;
	
	void print();
	
	friend class iterator;
	
	class iterator
	{
		public:
		
		iterator(std::array<index, N>& li, vertexID x) : currentNode(x), list(li) {}
		
		iterator& operator++()
		{
			if (currentNode != EMPTY)
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
		
		vertexID operator*() { return currentNode; }
		
		private:
		
		vertexID currentNode;
		const std::array<index, N>& list;
	};
	
	iterator begin() { return iterator(list,head); }
	iterator end  () { return iterator(list,EMPTY); }
	
	template <std::size_t X>
	friend void swap(indexedList<X>&,indexedList<X>&);
	
	private:
	
	struct index
	{
		bool inList;
		vertexID next, prev;
		
		index() : inList(false), next(EMPTY), prev(EMPTY) {}
	};
	
	unsigned numItems;
	
	std::array<index, N> list;
	
	vertexID head, tail;
};

#include "indexedList.tpp"

#endif
