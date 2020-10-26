#include "defs.hpp"
#include "subTree.hpp"
#include "indexedList.hpp"

#include <stack>
#include <iostream>

// Performs the bulk of the algorithm described in the paper.
void branch(int id, Subtree& S, indexedList<defs::vertexID, Graph::numVertices>& border,
	std::stack<defs::action>& previous_actions)
{
	// We only consider subtrees without children to be good candidates,
	// since any children of this tree would be better candidates.
	if (border.empty())
	{
		if (S.numInduced > defs::largestTree)
		{
			defs::checkCandidate(S);
		}
		++defs::numLeaves[id];
	}
	else
	{
		do
		{
			// Get and remove the first element
			defs::vertexID x = border.pop_front();
			
			// Push it onto a temporary list. This is a fix
			// to the base algorithm, it will not work without this
			// (along with the swap below)
			defs::lists[id][S.numInduced].push_back(x);
			
			// Ensure the addition would be valid
			if(S.add(x))
			{
				previous_actions.push({defs::stop,0});
				defs::update(S,border,x,previous_actions);
				
				if (defs::pool.n_idle() != 0)
				{
					defs::pool.push(branch,S,border,previous_actions);
				}
				else
				{
					branch(id,S,border,previous_actions);
				}
				
				defs::restore(border,previous_actions);
				
				S.rem(x);
			}
		}
		while (!border.empty());
		
		swap(border, defs::lists[id][S.numInduced]);
	}
}

int main(int num_args, char** args)
{
	if (num_args != 2)
	{
		std::cerr << "usage: " << args[0] << " <outfile>" << std::endl;
		exit(1);
	}
	
	defs::outfile = args[1];
	
	defs::start_time = clock();
	
	for (defs::vertexID x = 0; x < Graph::numVertices; x++)
	{
		// Makes a subgraph with one vertex, its root.
		Subtree S(x);
		
		indexedList<defs::vertexID, Graph::numVertices> border;
		
		std::stack<defs::action> previous_actions;
		
		defs::update(S,border,x,previous_actions);
		
		defs::pool.push(branch,S,border,previous_actions);
	}
	
	// Wait for all threads to finish
	while (defs::pool.n_idle() < defs::NUM_THREADS)
	{
		std::this_thread::sleep_for (std::chrono::seconds(1));
		
		unsigned long long total = 0;
		for (int i = 0; i < defs::NUM_THREADS; i++) total += defs::numLeaves[i];
		
		defs::IOmutex.lock();
		
		std::clog << "\r" << defs::threadSeconds() << " thread-seconds elapsed, "
			<< total << " leaves encountered" << std::flush;
		
		defs::lastWasNew = false;
		
		defs::IOmutex.unlock();
	}
	
	std::clog << std::endl << "Largest size = " << defs::largestTree << std::endl;
}
