#include "config.hpp"
#include "enumerate_subtrees.hpp"

#include <iostream>
#include <mutex>

namespace detail {
constexpr std::array<std::size_t, 3> dims{2, 3, 7};

std::ostream &operator<<(std::ostream &stream, const subtree_type &sub) {
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

} // namespace detail

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
  graph_type graph{detail::dims};

  // vertex_id max_size = 0;
  // for (const auto &sub : enumerate(graph)) {
  //   if (sub.n_induced() > max_size) {
  //     max_size = sub.n_induced();
  //     std::cout << "New max = " << max_size << '\n';
  //   }
  // }

  // std::mutex iomut;
  std::atomic_int count = 0;
  enumerate_recursive(graph, [&count](const subtree_type &) {
    // std::scoped_lock lock(iomut);
    // std::cout << sub << '\n';
    //++count;
  });
  std::cout << "Total: " << count << '\n';
}
