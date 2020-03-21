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

enum action_type
{
	add,
	rem,
	stop
};

struct action
{
	action_type type;
	vertexID v;
};

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
std::vector<std::array<indexedList<numVertices>, numVertices>> lists(NUM_THREADS);

std::vector<unsigned long long> numLeaves(NUM_THREADS);
bool lastWasNew = false;

// Returns the number of thread-seconds since the start of the program.
float threadSeconds()
{
	return (float)(clock()-start_time)/(CLOCKS_PER_SEC);
}

// Updates the border of S after adding x.
void update(Subtree& S, indexedList<numVertices>& border,
	vertexID x, std::stack<action>& previous_actions)
{
	for (vertexID y : G.vertices[x].neighbors)
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

// Restores the border of S after removing x.
void restore(indexedList<numVertices>& border,
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
unsigned randomBranch(int id, Subtree S, indexedList<numVertices> border)
{
	while(!border.empty())
	{
		vertexID x;
		do
		{
			// Get and remove a random element,
			// ensure it is valid.
			x = border.removeRandom();
		}
		while (!S.safeToAdd(x) && !border.empty());
		
		// Check for this, not the empty border.
		if (!S.add(x)) break;
		
		// Shortened version of update
		for (vertexID y : G.vertices[x].neighbors)
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
	
	if (S.numInduced > largestTree)
	{
		checkCandidate(S);
	}
	++numLeaves[id];
	
	return S.numInduced;
}

unsigned nested_monte_carlo(int id, Subtree& S, indexedList<numVertices>& border,
	std::stack<action>& previous_actions, unsigned level)
{
	unsigned best = 0;
	
	// Keep track of the vertices added.
	std::stack<vertexID> added;
	
	while(true)
	{
		for (auto iter = border.begin(); iter != border.end(); ++iter)
		{
			vertexID x = *iter;
			// No need to re-add these, since we will only going further down the tree.
			
			// ?
			if (!S.safeToAdd(x)) border.remove(x);
		}
		
		if (border.empty())
		{
			if (S.numInduced > best) best = S.numInduced;
			break;
		}
		
		unsigned nextVertex = -1;
		unsigned nextVertexScore = 0;
		if (level == 0)
		{
			do
			{
				// Get and remove the first element
				vertexID x = border.pop_front();
				
				// Push it onto a temporary list. This is a fix
				// to the base algorithm, it will not work without this
				// (along with the swap below)
				lists[id][S.numInduced].push_back(x);
				
				// All additions are valid, so no need to check.
				S.add(x);
				
				previous_actions.push({stop,0});

				update(S,border,x,previous_actions);
				
				// Give a copy to the call
				unsigned result = randomBranch(id,Subtree(S),indexedList<numVertices>(border));
				
				if (result > nextVertexScore)
				{
					nextVertex = x;
					nextVertexScore = result;
				}
				
				restore(border,previous_actions);
				
				S.rem(x);
			}
			while (!border.empty());
			
			std::swap(border, lists[id][S.numInduced]);
			
			//std::cout << "Decided on vertex " << nextVertex << std::endl;
		}
		else
		{
			//std::cout << border.size() << std::endl;
			do
			{
				// Get and remove the first element
				vertexID x = border.pop_front();
				
				// Push it onto a temporary list. This is a fix
				// to the base algorithm, it will not work without this
				// (along with the swap below)
				lists[id][S.numInduced].push_back(x);
				
				// All additions are valid, so no need to check.
				S.add(x);
				
				previous_actions.push({stop,0});
				update(S,border,x,previous_actions);
				
				unsigned result = nested_monte_carlo(id,S,border,previous_actions, level - 1);
				
				//std::cout << "Using vertex " << x << ", result = " << result << std::endl;
				
				if (result > nextVertexScore)
				{
					nextVertex = x;
					nextVertexScore = result;
				}
				
				restore(border,previous_actions);
				
				S.rem(x);
			}
			while (!border.empty());
			
			std::swap(border, lists[id][S.numInduced]);
		}
		
		S.add(nextVertex);
		
		added.push(nextVertex);
		
		border.remove(nextVertex);
		
		previous_actions.push({stop,0});
		update(S,border,nextVertex,previous_actions);
		
		if (level > 1)
		{
			std::cout << "Level " << level << " decided on vertex "
				<< nextVertex << ", numInduced = " << S.numInduced << 
				": " << threadSeconds() << std::endl;
		}
		
		if (S.numInduced > best)
		{
			best = S.numInduced;
		}
	}
	
	while(!added.empty())
	{
		vertexID x = added.top();
		added.pop();
		
		S.rem(x);
		
		restore(border,previous_actions);
		
		border.push_back(x);
	}
	
	return best;
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
	
	Subtree S(0);
	
	indexedList<numVertices> border;
	
	std::stack<action> previous_actions;
	
	update(S,border,0,previous_actions);
	
	nested_monte_carlo(0,S,border,previous_actions,2);
	
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
	
	std::clog << threadSeconds() << " thread-seconds" << std::endl;
	
	std::clog << "Largest size = " << largestTree << std::endl;
}
