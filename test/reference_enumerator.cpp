#include "reference_enumerator.hpp"

#include <algorithm>
#include <bit>
#include <queue>
#include <ranges>

namespace {
/**
 * @brief Checks if inducing a set of vertices produces a connected, acyclic
 * graph.
 * @param graph the reference graph
 * @param vertices the vertices to induce, effectively a dynamic bitset with
 * graph.vertices.size() elements, one per vertex.
 * @return true iff vertices represents an induced subtree
 */
bool is_connected_subtree(const graph_type &graph,
                          const std::size_t vertices_bitset) {
  if (vertices_bitset == 0) {
    // This will cause an assertion error later, so we short circuit this case.
    return true;
  }

  // Performs a BFS on the graph.

  const auto induced = [&vertices_bitset](auto i) {
    return (vertices_bitset & (1ull << i)) != 0;
  };

  // Find any vertex to start the search
  const auto start_id = *std::ranges::find_if(
      std::views::iota(0ull, graph.vertices.size()), induced);

  assert(start_id < graph.vertices.size());

  struct edge {
    vertex_id to, from;
  };

  // Stores the list of vertices to be searched next
  std::queue<edge> vertex_queue;
  vertex_queue.emplace(static_cast<vertex_id>(start_id), -1);

  std::size_t vertices_visited_bitset = 0;
  while (!vertex_queue.empty()) {
    const auto [id, from] = vertex_queue.front();
    vertex_queue.pop();

    // If this vertex has already been visited, then we have a cycle.
    if ((vertices_visited_bitset & (1ull << id)) != 0) {
      return false;
    }

    // Mark the vertex as visited
    vertices_visited_bitset |= (1ull << id);

    for (const auto &neighbor : graph.vertices[id].neighbors) {
      if (neighbor != from && induced(neighbor)) {
        vertex_queue.emplace(neighbor, id);
      }
    }
  }

  return vertices_visited_bitset == vertices_bitset;
}

subtree_type construct_snapshot(const graph_type &graph,
                                const std::size_t vertices_bitset) {
  const auto n_vertices = graph.vertices.size();

  subtree_type result{graph};

  for (vertex_id id = 0; id < n_vertices; ++id) {
    if (vertices_bitset & (1ull << id)) {
      result.add(id);
    }
  }

  return result;
}
} // namespace

namespace testing {
cppcoro::recursive_generator<subtree_type>
brute_force_enumerate(const graph_type &graph) {
  const auto n_vertices = graph.vertices.size();

  // Each iteration represents one potential induced subtree
  for (std::size_t i = 0; i < (1ull << n_vertices); ++i) {
    if (is_connected_subtree(graph, i)) {
      co_yield construct_snapshot(graph, i);
    }
  }
}
} // namespace testing