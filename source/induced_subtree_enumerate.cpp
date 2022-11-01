#include "config.hpp"
#include "enumerate_subtrees.hpp"

#include <iostream>

std::ostream &operator<<(std::ostream &stream, const subtree_type &sub) {
  const auto &dims = sub.base().dims_array;
  if (dims.size() == 1) {
    const vertex_id d1 = dims[0];
    for (vertex_id i = 0; i < d1; ++i) {
      stream << (sub.has(i) ? 'X' : '_');
    }
    stream << '\n';
  } else if (dims.size() == 2) {
    const vertex_id d1 = dims[0];
    const vertex_id d2 = dims[1];

    vertex_id index = 0;
    for (vertex_id i = 0; i < d1; ++i) {
      for (vertex_id j = 0; j < d2; ++j) {
        stream << (sub.has(index) ? 'X' : '_');
        ++index;
      }
      stream << '\n';
    }
  } else {
    // TODO
  }
  return stream;
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  graph_type graph{3, 2, 2};

  vertex_id max_size = 0;
  for (const auto &sub : enumerate(graph)) {
    if (sub.n_induced() > max_size) {
      max_size = sub.n_induced();
      std::cout << "New max = " << max_size << '\n';
    }
  }
}
