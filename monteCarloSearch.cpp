#include "subTree.hpp"
#include "indexedList.hpp"
#include "CTPL/ctpl_stl.h"

#include <stack>
#include <iostream>
#include <ctime>
#include <thread>
#include <mutex>
#include <random>
#include <stack>

#define NUM_THREADS (std::thread::hardware_concurrency())

// Since there is only one base graph, we can let it be global.
const Graph G;

// Thread pool
ctpl::thread_pool pool(NUM_THREADS);

// Maximum size graph seen so far
unsigned largestTree = 0, largestWithEnclosed = 0;

// File to write the best graph seen so far to
std::string outfile;

// The start time of the program
clock_t start_time;

// Grid of indexedLists, used to store the border elements as they are removed,
// then swapped back to restore. A call to branch can find the list it should
// use by going to lists[id][S.numInduced].
std::vector<std::array<indexedList<defs::numVertices>, defs::numVertices>> lists(NUM_THREADS);

std::vector<unsigned long long> numLeaves(NUM_THREADS);
bool lastWasNew = false;

// Returns the number of thread-seconds since the start of the program.
float threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

// Updates the border of S after adding x.
void update(Subtree& S, indexedList<defs::numVertices>& border,
	defs::vertexID x, std::stack<defs::action>& previous_actions)
{
	for (defs::vertexID y : G.vertices[x].neighbors)
	{
		// Pushes the current action, will need
		// to do the opposite action to reverse.
		if (S.cnt(y) > 1)
		{
			// This is a fix for the base algorithm, it will
			// not work without this.
			if (border.remove(y))
			{
				previous_actions.push({defs::rem,y});
			}
		}
		else if (y > S.root && !S.has(y))
		{
			border.push_front(y);
			previous_actions.push({defs::add,y});
		}
	}
}

// Updates the border of S after adding x, does not track changes.
void simpleUpdate(Subtree& S, indexedList<defs::numVertices>& border, defs::vertexID x)
{
	for (defs::vertexID y : G.vertices[x].neighbors)
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

// Restores the border of S after removing x.
void restore(indexedList<defs::numVertices>& border,
	std::stack<defs::action>& previous_actions)
{
	while (true)
	{
		defs::action act = previous_actions.top();
		previous_actions.pop();
		
		if (act.type == defs::stop)
		{
			return;
		}
		
		if (act.type == defs::add)
		{
			border.remove(act.v);
		}
		else /* act.type == rem */
		{
			border.push_front(act.v);
		}
	}
}

// After confirming S has a greater number of blocks than seen before,
// prints to clog If S does not have enclosed space, updates
// largestTree and writes the result to outfile, does neither if S
// does have enclosed space. Relies on the mutex below for thread safety.

std::mutex mutex;

void checkCandidate(Subtree S)
{	
	mutex.lock();
	
	if (S.numInduced > largestTree)
	{
		if (!lastWasNew)
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
			
			if (largestWithEnclosed < largestTree) largestWithEnclosed = largestTree;
			
			S.writeToFile(outfile);
			
			std::clog << largestTree << " vertices, found at " <<
				threadSeconds() << " thread-seconds" << std::endl;
		}
	}
	
	mutex.unlock();
}

// Randomly adds vertices to S until it becomes maximal, then returns
// its size.
// Current path should start with only the last added vertex.
void randomBranch(int id, Subtree S, indexedList<defs::numVertices> border, unsigned& bestResult,
	indexedList<defs::numVertices> currentPath, indexedList<defs::numVertices>& bestPath)
{
	while(!border.empty())
	{
		defs::vertexID x;
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
	
	if (S.numInduced > largestTree)
	{
		checkCandidate(S);
	}
	++numLeaves[id];
	
	if (S.numInduced > bestResult)
	{
		bestResult = S.numInduced;
		std::swap(bestPath, currentPath);
	}
}

void nested_monte_carlo(int id, Subtree& S, indexedList<defs::numVertices>& border,
	std::stack<defs::action>& previous_actions, unsigned level, unsigned& globalBestResult,
	indexedList<defs::numVertices> currentPath, indexedList<defs::numVertices>& globalBestPath)
{
	// Keep track of the vertices added.
	std::stack<defs::vertexID> added;
	
	indexedList<defs::numVertices> bestPath;
	unsigned bestResult = 0;
	while(true)
	{
		// Temporarily remove any vertices that are invalid to add.
		for (defs::vertexID x : border)
		{
			if (!S.safeToAdd(x))
			{
				border.remove(x);
				lists[id][S.numInduced].push_back(x);
			}
		}
		
		if (border.empty())
		{
			std::swap(border, lists[id][S.numInduced]);
			break;
		}
		
		indexedList<defs::numVertices> trialPath;
		do
		{
			// Get and remove the first element
			defs::vertexID x = border.pop_front();
			
			// Push it onto a temporary list. This is a fix
			// to the base algorithm, it will not work without this
			// (along with the swap below)
			lists[id][S.numInduced].push_back(x);
			
			// All additions are valid, so no need to check.
			S.add(x);
			
			previous_actions.push({defs::stop,0});
			
			update(S,border,x,previous_actions);
			
			trialPath.push_back(x);
			
			if (level == 0)
				randomBranch(id,S,border,bestResult,trialPath,bestPath);
			else
				nested_monte_carlo(id,S,border,previous_actions, level - 1,
					bestResult,trialPath,bestPath);
			
			trialPath.pop_back();
			
			restore(border,previous_actions);
			
			S.rem(x);
		}
		while (!border.empty());
			
		std::swap(border, lists[id][S.numInduced]);
		
		defs::vertexID nextVertex = bestPath.pop_front();
		
		S.add(nextVertex);
		
		added.push(nextVertex);
		
		border.remove(nextVertex);
		
		previous_actions.push({defs::stop,0});
		update(S,border,nextVertex,previous_actions);
		
		if (level == NMC_LEVEL)
		{
			std::cout << "Level " << level << " decided on vertex "
				<< nextVertex << ", numInduced = " << S.numInduced << 
				": " << threadSeconds() << std::endl;
		}
	}
	
	unsigned result = S.numInduced;
	
	// There is one item, temporarily remove it so
	// we can put the rest of the items in order.
	defs::vertexID temp = currentPath.pop_front();
	while(!added.empty())
	{
		defs::vertexID x = added.top();
		added.pop();
		
		S.rem(x);
		
		restore(border,previous_actions);
		
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
	
	outfile = args[1];
	
	srand(time(NULL));
	start_time = clock();
	
	unsigned globalBestResult = 0;
	indexedList<defs::numVertices> currentPath;
	currentPath.push_front(0);
	indexedList<defs::numVertices> globalBestPath;
	
	Subtree S(0);
	
	indexedList<defs::numVertices> border;
	
	std::stack<defs::action> previous_actions;
	
	update(S,border,0,previous_actions);
	
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
		
		mutex.lock();
		
		std::cout << "\r" << threadSeconds() << " thread-seconds elapsed, "
			<< total << " leaves encountered";
		
		std::cout.flush();
		
		lastWasNew = false;
		
		mutex.unlock();
	}
	*/
	
	//std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size (no enclosed space) = " << largestTree << std::endl;
}
