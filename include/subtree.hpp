#pragma once

#include "graph.hpp"
#include "static_graph.hpp"

#include <vector>
#include <array>
#include <algorithm>

// Each index is either enabled or disabled, and includes its
// effective degree (which is cnt)
template<class graph_t>
struct subtree_vertex
{
	bool induced{false};
	graph_t::vertex_id effective_degree{0};
};

// Use the same type of container as was used in graph_t, but with
// subtree::vertex instead of graph_t::vertex.

template<class graph_t>
struct vertices_base
{
	std::vector<subtree_vertex<graph_t>> vertices;
	
	vertices_base(std::size_t size) : vertices(size) {}
};

template<std::size_t... dims>
struct vertices_base<static_hrp_graph<dims...>>
{
	std::array<subtree_vertex<static_hrp_graph<dims...>>, (dims * ...)> vertices;
	
	vertices_base(std::size_t) {}
};

// Represents an induced subtree
template<class graph_t>
class subtree : vertices_base<graph_t>
{
	graph_t::vertex_id n_induced;
	
	graph_t::vertex_id root;
	
	const graph_t base_graph;
	
	public:
	
	subtree(graph_t base, graph_t::vertex_id root_id)
		: vertices_base<graph_t>(base.vertices.size())
		, n_induced{0}, root{root_id}, base_graph{std::move(base)} {}
	
	graph_t::vertex_id cnt (graph_t::vertex_id i) const
		{ return vertices_base<graph_t>::vertices[i].effective_degree; }
	
	bool has (graph_t::vertex_id i) const
		{ return vertices_base<graph_t>::vertices[i].induced; }
	
	bool exists(graph_t::vertex_id i) const
		{ return i != graph_t::no_vertex && has(i); }
	
	// Assumes i is on the border of the current graph.
	// Returns true iff the vertex was added.
	// Currently, does not check for enclosed space.
	bool add(graph_t::vertex_id i)
	{
		vertices_base<graph_t>::vertices[i].induced = true;
		
		// This should have exactly one neighbor, we need to validate it
		auto neighbor = *std::ranges::find_if(base_graph.vertices[i].neighbors, has);
		
		++vertices_base<graph_t>::vertices[neighbor].effective_degree;
		
		if (validate(neighbor))
		{
			++n_induced;
			vertices_base<graph_t>::vertices[i].effective_degree = 1;
			return true;
		}
		else
		{
			// Undo changes made and report that this is invalid
			--vertices_base<graph_t>::vertices[neighbor].effective_degree;
			vertices_base<graph_t>::vertices[i].induced = false;
			return false;
		}
	}
	
	// Assumes i is induced and has exactly one neighbor, is intended to be
	// paired with add().
	void rem(graph_t::vertex_id i)
	{
		--n_induced;
		
		vertices_base<graph_t>::vertices[i].induced = false;
		
		--vertices_base<graph_t>::vertices[i].effective_degree;
		--std::ranges::find_if(base_graph.vertices[i].neighbors, has)->effective_degree;
	}
	
	// Defined only for dimension 3. A vertex is valid
	// if it has at most one axis with both neighbors.
	bool validate(graph_t::vertex_id i) const
	{
		if (base_graph.dims_array.size() == 3)
		{
			if (cnt(i) != 4) return cnt(i) < 4;
		}
		else
		{
			return true;
		}
	}
	
	// Returns true iff there is at least one block whose
	// faces cannot be accessed externally.
	//bool hasEnclosedSpace() const;
};
