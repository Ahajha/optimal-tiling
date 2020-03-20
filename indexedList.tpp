#include "indexedList.hpp"

#include <random>
#include <iostream>

template <std::size_t N>
indexedList<N>::indexedList() : numItems(0), list(), head(EMPTY), tail(EMPTY) {}

template <std::size_t N>
bool indexedList<N>::remove(int x)
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
void indexedList<N>::push_front(int x)
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
void indexedList<N>::push_back(int x)
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
int indexedList<N>::pop_front()
{
	int x = head;
	remove(head);
	return x;
}

template <std::size_t N>
int indexedList<N>::pop_back()
{
	int x = tail;
	remove(tail);
	return x;
}

template <std::size_t N>
bool indexedList<N>::empty() const
{
	return head == EMPTY;
}

template <std::size_t N>
unsigned indexedList<N>::size() const
{
	return numItems;
}

template <std::size_t N>
int indexedList<N>::removeRandom()
{
	int toRemove = rand() % numItems;
	
	int valueToRemove = head;
	for (unsigned i = 0; i < toRemove; i++)
		valueToRemove = list[valueToRemove].next;
	
	remove(valueToRemove);
	
	return valueToRemove;
}

template <std::size_t N>
void indexedList<N>::print()
{
	/*
	for (int x = head; x != EMPTY; x = list[x].next)
	{
		std::cout << x << " ";
	}
	std::cout << std::endl;
	*/
	for (unsigned i = 0; i < N; i++)
	{
		std::cout << "vertex " << i << ", " << (list[i].inList ? "in" : "not in") << ", "
			<< "next = " << list[i].next << ", prev = " << list[i].prev << std::endl;
	}
	std::cout << "head = " << head << ", tail = " << tail
		<< ", size = " << numItems << std::endl;
}
