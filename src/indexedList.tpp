#include "indexedList.hpp"

#include <random>

template<class T, T N>
constexpr indexedList<T,N>::indexedList() :
	numItems(0), list(), head(EMPTY), tail(EMPTY) {}

template<class T, T N>
constexpr bool indexedList<T,N>::remove(T x)
{
	if (!list[x].inList) return false;
	
	list[x].inList = false;
	
	// Removals are 'lazy', they do not reset the cell that was removed.
	if (head == x)
	{
		head = list[x].next;
	}
	else
	{
		list[list[x].prev].next = list[x].next;
	}
	
	if (tail == x)
	{
		tail = list[x].prev;
	}
	else
	{
		list[list[x].next].prev = list[x].prev;
	}
	
	--numItems;
	
	return true;
}

template<class T, T N>
constexpr void indexedList<T,N>::push_front(T x)
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

template<class T, T N>
constexpr void indexedList<T,N>::push_back(T x)
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

template<class T, T N>
constexpr T indexedList<T,N>::pop_front()
{
	T x = head;
	remove(head);
	return x;
}

template<class T, T N>
constexpr T indexedList<T,N>::pop_back()
{
	T x = tail;
	remove(tail);
	return x;
}

template<class T, T N>
constexpr bool indexedList<T,N>::empty() const
{
	return head == EMPTY;
}

template<class T, T N>
constexpr void swap(indexedList<T,N>& list1, indexedList<T,N>& list2)
{
	constexpr T EMPTY = indexedList<T,N>::EMPTY;

	// Loop follows the second list, since this constantly swaps the
	// next item information into the other list.
	for (T x = list1.head; x != EMPTY; x = list2.list[x].next)
	{
		std::swap(list1.list[x],list2.list[x]);
	}
	
	// Now follow list2 again, but only swap if list1 did not already swap.
	// The cell has been swapped if either the cell in list 2 is not induced
	// (since it should be) or if the cell in both lists are induced.
	// (!list2.list[x].inList || list1.list[x].inList), which negates to
	// (list2.list[x].inList && !list1.list[x].inList).
	// After the (potential) swap, the next cell will always be in list1.
	for (T x = list2.head; x != EMPTY; x = list1.list[x].next)
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

template<class T, T N>
constexpr bool indexedList<T,N>::exists(T x) const
{
	return list[x].inList;
}

template<class T, T N>
constexpr void indexedList<T,N>::clear()
{
	head = tail = EMPTY;
}

template<class T, T N>
constexpr T indexedList<T,N>::size() const
{
	return numItems;
}

template<class T, T N>
constexpr T indexedList<T,N>::removeRandom()
{
	T toRemove = rand() % numItems;
	
	T valueToRemove = head;
	for (T i = 0; i < toRemove; i++)
		valueToRemove = list[valueToRemove].next;
	
	remove(valueToRemove);
	
	return valueToRemove;
}
