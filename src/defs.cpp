#include "defs.hpp"

#include <iostream>

void defs::update(const Subtree& S, indexedList<Graph::vertexID, Graph::numVertices>& border,
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
		
		switch (act.type)
		{
			case add:
				border.remove(act.v);
				break;
			case rem:
				border.push_front(act.v);
				break;
			case stop:
				return;
		}
	}
}

float defs::threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

void defs::checkCandidate(const Subtree& S)
{
	std::lock_guard<std::mutex> lock(IOmutex);
	
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
}
