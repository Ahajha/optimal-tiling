#ifndef INDEXED_LIST_HPP
#define INDEXED_LIST_HPP

#include <array>
#include "defs.hpp"

/*
An indexedList is an array/doubly linked list hybrid,
with the benefits of both. It requires a fixed set of
integers 0-N (excluding N) to be the only candidates
for the list, without repeats. All insertions and
removals are in constant time.

T is the type used for indexing.
*/

template<class T, T N>
class indexedList
{
	private:
	
	struct index;
	
	public:
	
	[[nodiscard]] constexpr indexedList();
	
	// Returns true if x was removed, and false if x did not already exist here.
	// Remove and both pop methods assume there is at least one item in the list.
	constexpr bool remove(T);
	
	constexpr void push_front(T);
	constexpr void push_back (T);
	
	constexpr T pop_front ();
	constexpr T pop_back  ();
	
	[[nodiscard]] constexpr bool empty() const;
	
	[[nodiscard]] constexpr bool exists(T) const;
	
	constexpr void clear();
	
	// Removes a random item from the list and returns it, uniformly distributed.
	// Assumes there is an item to remove.
	constexpr T removeRandom();
	
	[[nodiscard]] constexpr T size() const;
	
	void print() const;
	
	friend class iterator;
	
	class iterator
	{
		public:
		
		[[nodiscard]] constexpr iterator(std::array<index, N>& li, T x) :
			currentNode(x), list(li) {}
		
		constexpr iterator& operator++()
		{
			if (currentNode != defs::EMPTY)
				currentNode = list[currentNode].next;
			return *this;
		}
		
		constexpr iterator  operator++(int)
		{
			iterator it = *this;
			++*this;
			return it;
		}
		
		[[nodiscard]] constexpr bool operator==(const iterator& i)
		{
			return currentNode == i.currentNode;
		}
		
		[[nodiscard]] constexpr T operator*() { return currentNode; }
		
		private:
		
		T currentNode;
		const std::array<index, N>& list;
	};
	
	[[nodiscard]] constexpr iterator begin() { return iterator(list, head); }
	[[nodiscard]] constexpr iterator end  () { return iterator(list, defs::EMPTY); }
	
	template<class T_, T_ N_>
	constexpr friend void swap(indexedList<T_,N_>&, indexedList<T_,N_>&);
	
	private:
	
	struct index
	{
		bool inList;
		T next, prev;
		
		[[nodiscard]] constexpr index() :
			inList(false), next(defs::EMPTY), prev(defs::EMPTY) {}
	};
	
	unsigned numItems;
	
	std::array<index, N> list;
	
	T head, tail;
};

#include "indexedList.tpp"

#endif
