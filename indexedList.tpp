#include "indexedList.hpp"

#include <random>
#include <iostream>

template <std::size_t N>
indexedList<N>::indexedList() : numItems(0), list(), head(EMPTY), tail(EMPTY) {}

template <std::size_t N>
bool indexedList<N>::remove(vertexID x)
{
	if (!list[x].inList) return false;
	
	list[x].inList = false;
	
	if (list[x].next != EMPTY)
		list[list[x].next].prev = list[x].prev;
	else // Tail of the list
		tail = list[x].prev;
	
	if (list[x].prev != EMPTY)
		list[list[x].prev].next = list[x].next;
	else // Head of the list
		head = list[x].next;
	
	//list[x].next = EMPTY;
	//list[x].prev = EMPTY;
	
	--numItems;
	
	return true;
}

template <std::size_t N>
void indexedList<N>::push_front(vertexID x)
{
	list[x].inList = true;
	list[x].next = head;
	list[x].prev = EMPTY;
	
	if (head == EMPTY)
	{
		tail = x;
	}
	else
	{
		list[head].prev = x;
	}
	
	head = x;
	
	++numItems;
}

template <std::size_t N>
void indexedList<N>::push_back(vertexID x)
{
	list[x].inList = true;
	list[x].next = EMPTY;
	list[x].prev = tail;
	
	if (tail == EMPTY)
	{
		head = x;
	}
	else
	{
		list[tail].next = x;
	}
	
	tail = x;
	
	++numItems;
}

template <std::size_t N>
vertexID indexedList<N>::pop_front()
{
	vertexID x = head;
	remove(head);
	return x;
}

template <std::size_t N>
vertexID indexedList<N>::pop_back()
{
	vertexID x = tail;
	remove(tail);
	return x;
}

template <std::size_t N>
bool indexedList<N>::empty() const
{
	return head == EMPTY;
}

template <std::size_t N>
void swap(indexedList<N>& list1, indexedList<N>& list2)
{
	// Loop follows the second list, since this constantly swaps the
	// next item information into the other list.
	for (int x = list1.head; x != EMPTY; x = list2.list[x].next)
	{
		std::swap(list1.list[x],list2.list[x]);
	}
	
	// Now follow list2 again, but only swap if list1 did not already swap.
	// The cell has been swapped if either the cell in list 2 is not induced
	// (since it should be) or if the cell in both lists are induced.
	// (!list2.list[x].inList || list1.list[x].inList), which negates to
	// (list2.list[x].inList && !list1.list[x].inList).
	// After the (potential) swap, the next cell will always be in list1.
	for (int x = list2.head; x != EMPTY; x = list1.list[x].next)
	{
		if (list2.list[x].inList && !list1.list[x].inList)
		{
			std::swap(list1.list[x],list2.list[x]);
		}
	}
	
	// Finally, swap the heads and tails.
	std::swap(list1.head,list2.head);
	std::swap(list1.tail,list2.tail);
}

template <std::size_t N>
bool indexedList<N>::exists(vertexID x) const
{
	return list[x].inList;
}

template <std::size_t N>
void indexedList<N>::clear()
{
	head = tail = EMPTY;
}

template <std::size_t N>
unsigned indexedList<N>::size() const
{
	return numItems;
}

template <std::size_t N>
vertexID indexedList<N>::removeRandom()
{
	vertexID toRemove = rand() % numItems;
	
	vertexID valueToRemove = head;
	for (unsigned i = 0; i < toRemove; i++)
		valueToRemove = list[valueToRemove].next;
	
	remove(valueToRemove);
	
	return valueToRemove;
}

template <std::size_t N>
void indexedList<N>::print()
{
	/*
	for (vertexID x = head; x != EMPTY; x = list[x].next)
	{
		std::cout << x << " ";
	}
	std::cout << std::endl;
	*/
	std::cout << "(default cells omitted)" << std::endl;
	for (unsigned i = 0; i < N; i++)
	{
		if (list[i].inList || list[i].next != EMPTY || list[i].prev != EMPTY)
			std::cout << "vertex " << i << ", " << (list[i].inList ? "in" : "not in") << ", "
				<< "next = " << list[i].next << ", prev = " << list[i].prev << std::endl;
	}
	std::cout << "head = " << head << ", tail = " << tail
		<< ", size = " << numItems << std::endl;
}
