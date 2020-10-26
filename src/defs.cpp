#include "defs.hpp"
#include "indexedList.hpp"
#include "subTree.hpp"
#include "graph.hpp"

#include <thread>
#include <iostream>

const int defs::NUM_THREADS = std::thread::hardware_concurrency();

ctpl::thread_pool defs::pool(NUM_THREADS);

unsigned defs::largestTree = 0, defs::largestWithEnclosed = 0;

std::string defs::outfile;

clock_t defs::start_time;

std::vector<
	std::array<indexedList<Graph::vertexID, Graph::numVertices>, Graph::numVertices>
> defs::lists(NUM_THREADS);

std::vector<unsigned long long> defs::numLeaves(NUM_THREADS);

bool defs::lastWasNew = false;

std::mutex defs::IOmutex;

void defs::update(Subtree& S, indexedList<Graph::vertexID, Graph::numVertices>& border,
	Graph::vertexID x, std::stack<action>& previous_actions)
{
	for (Graph::vertexID y : Graph::vertices[x].neighbors)
	{
		// Pushes the current action, will need
		// to do the opposite action to reverse.
		if (S.cnt(y) > 1)
		{
			// This is a fix for the base algorithm, it will
			// not work without this.
			if (border.remove(y))
			{
				previous_actions.push({rem,y});
			}
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
			previous_actions.push({add,y});
		}
	}
}

void defs::restore(indexedList<Graph::vertexID, Graph::numVertices>& border,
	std::stack<action>& previous_actions)
{
	while (true)
	{
		action act = previous_actions.top();
		previous_actions.pop();
		
		if (act.type == stop)
		{
			return;
		}
		
		if (act.type == add)
		{
			border.remove(act.v);
		}
		else /* act.type == rem */
		{
			border.push_front(act.v);
		}
	}
}

float defs::threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

void defs::checkCandidate(Subtree S)
{	
	IOmutex.lock();
	
	if (S.numInduced > largestTree)
	{
		if (!defs::lastWasNew)
		{
			std::cout << std::endl;
			lastWasNew = true;
		}
		
		if (S.hasEnclosedSpace())
		{
			if (S.numInduced > largestWithEnclosed)
			{
				largestWithEnclosed = S.numInduced;
				
				S.writeToFile(outfile + "_enclosed");
				
				std::clog << S.numInduced
					<< " vertices with enclosed space, found at "
					<< threadSeconds() << " thread-seconds" << std::endl;
			}
		}
		else
		{
			largestTree = S.numInduced;
			
			if (largestWithEnclosed < defs::largestTree)
				largestWithEnclosed = defs::largestTree;
			
			S.writeToFile(defs::outfile);
			
			std::clog << largestTree << " vertices, found at " <<
				threadSeconds() << " thread-seconds" << std::endl;
		}
	}
	
	IOmutex.unlock();
}
