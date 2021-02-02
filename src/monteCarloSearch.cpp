#include "defs.hpp"
#include "graph.hpp"
#include "subTree.hpp"
#include "indexedList.hpp"

#include <stack>
#include <iostream>
#include <random>

// Updates the border of S after adding x, does not track changes.
void simpleUpdate(Subtree& S,
	indexedList<Graph::vertexID, Graph::numVertices>& border, Graph::vertexID x)
{
	for (Graph::vertexID y : Graph::vertices[x].neighbors)
	{
		if (S.cnt(y) > 1)
		{
			border.remove(y);
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
		}
	}
}

// Randomly adds vertices to S until it becomes maximal, then returns
// its size.
// Current path should start with only the last added vertex.
void randomBranch(int id, Subtree S, indexedList<Graph::vertexID, Graph::numVertices> border,
	unsigned& bestResult, indexedList<Graph::vertexID, Graph::numVertices> currentPath,
	indexedList<Graph::vertexID, Graph::numVertices>& bestPath)
{
	while(!border.empty())
	{
		Graph::vertexID x;
		do
		{
			// Get and remove a random element,
			// ensure it is valid.
			x = border.removeRandom();
		}
		while (!S.safeToAdd(x) && !border.empty());
		
		// Check for this, not the empty border.
		if (!S.add(x)) break;
		
		simpleUpdate(S,border,x);
		
		currentPath.push_back(x);
	}
	
	if (S.numInduced > defs::largestTree)
	{
		defs::checkCandidate(S);
	}
	++defs::numLeaves[id];
	
	if (S.numInduced > bestResult)
	{
		bestResult = S.numInduced;
		std::swap(bestPath, currentPath);
	}
}

void nested_monte_carlo(int id, Subtree& S,
	indexedList<Graph::vertexID, Graph::numVertices>& border,
	std::stack<defs::action>& previous_actions, unsigned level, unsigned& globalBestResult,
	indexedList<Graph::vertexID, Graph::numVertices> currentPath,
	indexedList<Graph::vertexID, Graph::numVertices>& globalBestPath)
{
	// Keep track of the vertices added.
	std::stack<Graph::vertexID> added;
	
	indexedList<Graph::vertexID, Graph::numVertices> bestPath;
	unsigned bestResult = 0;
	while(true)
	{
		// Temporarily remove any vertices that are invalid to add.
		for (Graph::vertexID x : border)
		{
			if (!S.safeToAdd(x))
			{
				border.remove(x);
				defs::lists[id][S.numInduced].push_back(x);
			}
		}
		
		if (border.empty())
		{
			std::swap(border, defs::lists[id][S.numInduced]);
			break;
		}
		
		indexedList<Graph::vertexID, Graph::numVertices> trialPath;
		do
		{
			// Get and remove the first element
			Graph::vertexID x = border.pop_front();
			
			// Push it onto a temporary list. This is a fix
			// to the base algorithm, it will not work without this
			// (along with the swap below)
			defs::lists[id][S.numInduced].push_back(x);
			
			// All additions are valid, so no need to check.
			S.add(x);
			
			previous_actions.push({defs::stop,0});
			
			defs::update(S,border,x,previous_actions);
			
			trialPath.push_back(x);
			
			if (level == 0)
				randomBranch(id,S,border,bestResult,trialPath,bestPath);
			else
				nested_monte_carlo(id,S,border,previous_actions, level - 1,
					bestResult,trialPath,bestPath);
			
			trialPath.pop_back();
			
			defs::restore(border,previous_actions);
			
			S.rem(x);
		}
		while (!border.empty());
		
		std::swap(border, defs::lists[id][S.numInduced]);
		
		Graph::vertexID nextVertex = bestPath.pop_front();
		
		S.add(nextVertex);
		
		added.push(nextVertex);
		
		border.remove(nextVertex);
		
		previous_actions.push({defs::stop,0});
		defs::update(S,border,nextVertex,previous_actions);
		
		if (level == NMC_LEVEL)
		{
			std::cout << "Level " << level << " decided on vertex "
				<< static_cast<uintmax_t>(nextVertex) << ", numInduced = "
				<< S.numInduced << ": " << defs::threadSeconds() << std::endl;
		}
	}
	
	unsigned result = S.numInduced;
	
	// There is one item, temporarily remove it so
	// we can put the rest of the items in order.
	Graph::vertexID temp = currentPath.pop_front();
	while(!added.empty())
	{
		Graph::vertexID x = added.top();
		added.pop();
		
		S.rem(x);
		
		defs::restore(border,previous_actions);
		
		border.push_back(x);
		
		currentPath.push_front(x);
	}
	currentPath.push_front(temp);
	
	if (result > globalBestResult)
	{
		globalBestResult = result;
		std::swap(globalBestPath, currentPath);
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
	
	srand(time(NULL));
	defs::start_time = clock();
	
	unsigned globalBestResult = 0;
	indexedList<Graph::vertexID, Graph::numVertices> currentPath;
	currentPath.push_front(0);
	indexedList<Graph::vertexID, Graph::numVertices> globalBestPath;
	
	Subtree S(0);
	
	indexedList<Graph::vertexID, Graph::numVertices> border;
	
	std::stack<defs::action> previous_actions;
	
	defs::update(S,border,0,previous_actions);
	
	nested_monte_carlo(0,S,border,previous_actions,NMC_LEVEL,
		globalBestResult,currentPath,globalBestPath);
	
	std::cout << "Monte-Carlo result = " << globalBestResult << std::endl;
	
	//pool.push(randomSample,0);
	
	// Wait for all threads to finish
	/*
	while (pool.n_idle() < NUM_THREADS)
	{
		std::this_thread::sleep_for (std::chrono::seconds(1));
		
		unsigned long long total = 0;
		for (unsigned i = 0; i < NUM_THREADS; i++) total += numLeaves[i];
		
		std::lock_guard<std::mutex> lock(IOmutex);
		
		std::cout << "\r" << threadSeconds() << " thread-seconds elapsed, "
			<< total << " leaves encountered";
		
		std::cout.flush();
		
		lastWasNew = false;
	}
	*/
	
	//std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size (no enclosed space) = " << defs::largestTree << std::endl;
}
