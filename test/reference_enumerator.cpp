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
  // Performs a BFS on the graph. If a vertex has already been visited,

  const auto induced = [&vertices_bitset](auto i) {
    return (vertices_bitset & (1ull << i)) != 0;
  };

  // Find any vertex to start the search
  const auto start_id = *std::ranges::find_if(
      std::views::iota(0ull, graph.vertices.size()), induced);

  // Stores the list of vertices to be searched next
  std::queue<vertex_id> vertex_queue;
  vertex_queue.emplace(static_cast<vertex_id>(start_id));

  std::size_t vertices_visited_bitset = 0;
  while (!vertex_queue.empty()) {
    const auto id = vertex_queue.front();
    vertex_queue.pop();

    // If this vertex has already been visited, then we have a cycle.
    if ((vertices_visited_bitset & (1ull << id)) != 0) {
      return false;
    }

    // Mark the vertex as visited
    vertices_visited_bitset |= (1ull << id);

    for (const auto &neighbor : graph.vertices[id].neighbors) {
      if (induced(neighbor)) {
        vertex_queue.emplace(neighbor);
      }
    }
  }

  return vertices_visited_bitset == vertices_bitset;
}

subtree_snapshot construct_snapshot(const graph_type &graph,
                                    const std::size_t vertices_bitset) {
  const auto n_vertices = graph.vertices.size();

  subtree_snapshot snapshot;

  snapshot.n_induced = std::popcount(vertices_bitset);
  for (vertex_id id = 0; id < n_vertices; ++id) {
    if (vertices_bitset & (1ull << id)) {
      snapshot.cells[id].has = true;

      for (const auto &neighbor : graph.vertices[id].neighbors) {
        ++snapshot.cells[neighbor].count;
      }
    }
  }

  return snapshot;
}
} // namespace

namespace testing {
std::vector<subtree_snapshot> brute_force_enumerate(const graph_type &graph) {
  std::vector<subtree_snapshot> result;
  const auto n_vertices = graph.vertices.size();

  // Each iteration represents one potential induced subtree
  for (std::size_t i = 0; i < (1ull << n_vertices); ++i) {
    if (is_connected_subtree(graph, i)) {
      // Add the result
      result.push_back(construct_snapshot(graph, i));
    }
  }

  return result;
}
} // namespace testing