#ifndef INDEXED_LIST_HPP
#define INDEXED_LIST_HPP

#include <cstddef>
#include <array>

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
	public:
	
	indexedList();

	// Returns true if x was removed, and false if x did not already exist here.
	// Remove and both pop methods assume there is at least one item in the list.
	bool remove(int x);
	
	void push_front(int x);
	void push_back (int x);
	
	int  pop_front ();
	int  pop_back  ();
	
	bool empty();
	
	private:
	
	static constexpr int EMPTY = -1;
	
	struct index
	{
		bool inList;
		int next, prev;
		
		index() : inList(false), next(EMPTY), prev(EMPTY) {}
	};
	
	std::array<index, N> list;
	
	int head, tail;
};

#include "indexedList.tpp"

#endif
