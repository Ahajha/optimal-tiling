#pragma once

#include "graph.hpp"
#include "static_graph.hpp"

#include <algorithm>
#include <array>
#include <vector>

// Each index is either enabled or disabled, and includes its
// effective degree (which is cnt)
template <class graph_t> struct subtree_vertex {
  // Induced is whether the vertex is induced in the current subtree.
  bool induced{false};

  // Effective degree is the number of neighbors that are induced. Note
  // that this vertex being induced has no effect on this number.
  graph_t::vertex_id effective_degree{0};
};

// Use the same type of container as was used in graph_t, but with
// subtree::vertex instead of graph_t::vertex.

template <class graph_t> struct vertices_base {
  std::vector<subtree_vertex<graph_t>> vertices;

  vertices_base(std::size_t size) : vertices(size) {}
};

template <std::size_t... dims> struct vertices_base<static_hrp_graph<dims...>> {
  std::array<subtree_vertex<static_hrp_graph<dims...>>, (dims * ...)> vertices;

  vertices_base(std::size_t) {}
};

// Represents an induced subtree
template <class graph_t> class subtree : vertices_base<graph_t> {
  graph_t::vertex_id n_induced_;

  graph_t::vertex_id root;

  const graph_t base_graph;

public:
  subtree(graph_t base, graph_t::vertex_id root_id)
      : vertices_base<graph_t>(base.vertices.size()),
        n_induced_{1}, root{root_id}, base_graph{std::move(base)} {
    vertices_base<graph_t>::vertices[root].induced = true;

    for (const auto neighbor : base_graph.vertices[root].neighbors)
      ++vertices_base<graph_t>::vertices[neighbor].effective_degree;
  }

  // Returns the number of currently induced neighbors of vertex i.
  graph_t::vertex_id cnt(graph_t::vertex_id i) const {
    return vertices_base<graph_t>::vertices[i].effective_degree;
  }

  // Returns true iff vertex i is currently induced
  bool has(graph_t::vertex_id i) const {
    return vertices_base<graph_t>::vertices[i].induced;
  }

  // Returns true iff i is not the empty vertex ID and this graph contains i.
  bool exists(graph_t::vertex_id i) const {
    return i != graph_t::no_vertex && has(i);
  }

  // Returns the number of induced vertices.
  graph_t::vertex_id n_induced() const { return n_induced_; }

  // Assumes i is on the border of the current graph.
  // Returns true iff the vertex was added.
  // Currently, does not check for enclosed space.
  bool add(graph_t::vertex_id i) {
    // This should have exactly one neighbor, we need to validate it
    const auto sole_neighbor = sole_neighbor_of(i);

    if (validate(sole_neighbor)) {
      vertices_base<graph_t>::vertices[i].induced = true;
      ++n_induced_;

      for (const auto neighbor : base_graph.vertices[i].neighbors)
        ++vertices_base<graph_t>::vertices[neighbor].effective_degree;

      return true;
    } else {
      // Undo changes made and report that this is invalid
      vertices_base<graph_t>::vertices[i].induced = false;
      return false;
    }
  }

  // Assumes i is induced and has exactly one neighbor, is intended to be
  // paired with add().
  void rem(graph_t::vertex_id i) {
    vertices_base<graph_t>::vertices[i].induced = false;
    --n_induced_;

    for (const auto neighbor : base_graph.vertices[i].neighbors)
      --vertices_base<graph_t>::vertices[neighbor].effective_degree;
  }

private:
  // Defined only for dimension 3. A vertex is valid
  // if it has at most one axis with both neighbors.
  bool validate(graph_t::vertex_id i) const {
    if (base_graph.dims_array.size() == 3) {
      // !!!! This should be called when *considering* this vertex
      // to be induced, so this condition is really
      // if (cnt(i) != 4) return cnt(i) < 4;
      if (cnt(i) != 3)
        return cnt(i) < 3;

      auto &dirs = base_graph.vertices[i].directions;

      // Ensure all axis have at least one neighbor
      for (auto d = 0ul; d < 3ul; ++d) {
        // 5 - d gets the opposite direction
        if (!exists(dirs[d]) && !exists(dirs[5ul - d]))
          return false;
      }
      return true;
    } else {
      return true;
    }
  }

  // Assuming i has one neighbor, returns the ID of that neighbor.
  graph_t::vertex_id sole_neighbor_of(graph_t::vertex_id i) {
    return *std::ranges::find_if(base_graph.vertices[i].neighbors,
                                 [this](auto vid) { return has(vid); });
  }

  // Returns true iff there is at least one block whose
  // faces cannot be accessed externally.
  // bool hasEnclosedSpace() const;
};
