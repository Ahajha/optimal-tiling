#pragma once

#include "concepts.hpp"
#include "graph.hpp"
#include "permutation.hpp"
#include "static_graph.hpp"

#include <range/v3/algorithm/min.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <span>
#include <vector>

// Each index is either enabled or disabled, and includes its
// effective degree (which is cnt)
template <class graph_t> struct subtree_vertex {
  // Induced is whether the vertex is induced in the current subtree.
  bool induced{false};

  // Effective degree is the number of neighbors that are induced. Note
  // that this vertex being induced has no effect on this number.
  graph_t::vertex_id effective_degree{0};

  [[nodiscard]] constexpr auto
  operator<=>(const subtree_vertex &) const = default;
};

// Use the same type of container as was used in graph_t, but with
// subtree::vertex instead of graph_t::vertex.

template <class graph_t> struct vertices_base {
  std::vector<subtree_vertex<graph_t>> vertices;

  vertices_base(std::size_t size) : vertices(size) {}

  [[nodiscard]] auto operator<=>(const vertices_base &) const = default;
};

template <std::size_t... dims> struct vertices_base<static_hrp_graph<dims...>> {
  std::array<subtree_vertex<static_hrp_graph<dims...>>, (dims * ...)> vertices;

  vertices_base(std::size_t) {}

  [[nodiscard]] constexpr auto
  operator<=>(const vertices_base &) const = default;
};

// Represents an induced subtree
template <class graph_t> class subtree : public vertices_base<graph_t> {
  graph_t::vertex_id n_induced_;

  graph_t::vertex_id root_;

  const graph_t base_graph;

public:
  /**
   * @brief Construct a new subtree object with no root. The root is set to an
   * invalid state, so no operations should be performed on a subtree
   * constructed this way.
   * @param base The base graph that this is an induced subtree of
   */
  subtree(graph_t base)
      : vertices_base<graph_t>(base.vertices.size()),
        n_induced_{0}, root_{graph_t::no_vertex}, base_graph{std::move(base)} {}

  subtree(graph_t base, graph_t::vertex_id root_id)
      : vertices_base<graph_t>(base.vertices.size()),
        n_induced_{1}, root_{root_id}, base_graph{std::move(base)} {
    vertices_base<graph_t>::vertices[root_].induced = true;

    for (const auto neighbor : base_graph.vertices[root_].neighbors)
      ++vertices_base<graph_t>::vertices[neighbor].effective_degree;
  }

  subtree(
      graph_t base,
      detail::range_of_convertible_to<typename graph_t::vertex_id> auto &&verts)
      : vertices_base<graph_t>(base.vertices.size()),
        n_induced_{0}, root_{verts.empty() ? typename graph_t::vertex_id{0}
                                           : ranges::min(verts)},
        base_graph{std::move(base)} {
    for (const auto vert : verts) {
      add(vert);
    }
  }

  // Returns the base graph
  const graph_t &base() const { return base_graph; }

  // Returns the number of currently induced neighbors of vertex i.
  graph_t::vertex_id cnt(graph_t::vertex_id i) const {
    debug_bounds_check(i);
    return vertices_base<graph_t>::vertices[i].effective_degree;
  }

  // Returns true iff vertex i is currently induced
  bool has(graph_t::vertex_id i) const {
    debug_bounds_check(i);
    return vertices_base<graph_t>::vertices[i].induced;
  }

  // Returns true iff i is not the empty vertex ID and this graph contains i.
  bool exists(graph_t::vertex_id i) const {
    if (i != graph_t::no_vertex) {
      return false;
    }
    debug_bounds_check(i);
    return has(i);
  }

  // Returns the number of induced vertices.
  graph_t::vertex_id n_induced() const { return n_induced_; }

  // Adds i to the current subtree.
  // Assumes i is on the border of the current graph.
  // Currently, does not check for enclosed space.
  void add(graph_t::vertex_id i) {
    debug_bounds_check(i);

    vertices_base<graph_t>::vertices[i].induced = true;
    ++n_induced_;

    for (const auto neighbor : base_graph.vertices[i].neighbors) {
      ++vertices_base<graph_t>::vertices[neighbor].effective_degree;
    }
  }

  // Assumes i is induced and has exactly one neighbor, is intended to be
  // paired with add().
  void rem(graph_t::vertex_id i) {
    debug_bounds_check(i);
    assert(has(i));
    assert(cnt(i) == 1);

    vertices_base<graph_t>::vertices[i].induced = false;
    --n_induced_;

    for (const auto neighbor : base_graph.vertices[i].neighbors)
      --vertices_base<graph_t>::vertices[neighbor].effective_degree;
  }

  graph_t::vertex_id root() const { return root_; }

  [[nodiscard]] auto operator<=>(const subtree &other) const {
    return this->vertices <=> other.vertices;
  }

  subtree apply_permutation(
      const std::span<const typename graph_t::vertex_id> perm) const {
    subtree result{base_graph};
    result.n_induced_ = n_induced_;
    for (std::size_t i = 0; i < this->vertices.size(); ++i) {
      result.vertices[perm[i]] = this->vertices[i];
    }
    return result;
  }

private:
  // Defined only for dimension 3. A vertex is valid
  // if it has at most one axis with both neighbors.
  // TODO This is broken
  bool validate(graph_t::vertex_id i) const {
    if (base_graph.dims_array.size() == 3) {
      // !!!! This should be called when *considering* this vertex
      // to be induced, so this condition is really
      // if (cnt(i) != 4) return cnt(i) < 4;
      if (cnt(i) != 3) {
        return cnt(i) < 3;
      }

      const auto &dirs = base_graph.vertices[i].directions;

      // Ensure all axis have at least one neighbor
      for (auto d = 0ul; d < 3ul; ++d) {
        // 5 - d gets the opposite direction
        if (!exists(dirs[d]) && !exists(dirs[5ul - d])) {
          return false;
        }
      }
    }
    return true;
  }

  void debug_bounds_check([[maybe_unused]] graph_t::vertex_id i) const {
    if constexpr (std::unsigned_integral<typename graph_t::vertex_id>) {
      assert(0 <= i);
    }
    assert(i < base_graph.vertices.size());
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
