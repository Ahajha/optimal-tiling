#include "config.hpp"
#include "enumerate_subtrees.hpp"

#include <range/v3/view/drop.hpp>

#include <iostream>
#include <mutex>

namespace detail {

struct dim_subtree {
  std::span<const std::size_t> dims;
  const subtree_type &sub;
};

std::ostream &operator<<(std::ostream &stream, const dim_subtree &dim_sub) {
  const auto &[dims, sub] = dim_sub;
  if (dims.size() == 1) {
    const vertex_id d1 = static_cast<vertex_id>(dims[0]);
    for (vertex_id i = 0; i < d1; ++i) {
      stream << (sub.has(i) ? 'X' : '_');
    }
    stream << '\n';
  } else if (dims.size() == 2) {
    const vertex_id d1 = static_cast<vertex_id>(dims[0]);
    const vertex_id d2 = static_cast<vertex_id>(dims[1]);

    vertex_id index = 0;
    for (vertex_id i = 0; i < d2; ++i) {
      for (vertex_id j = 0; j < d1; ++j) {
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

int main(int argc, char *argv[]) {
  std::vector<std::size_t> dims;
  for (const auto arg_str : std::span{argv, static_cast<std::size_t>(argc)} |
                                ranges::views::drop(1)) {
    dims.push_back(static_cast<std::size_t>(std::stoi(arg_str)));
  }
  graph_type graph{dims};

  auto max_subtree = enumerate_recursive(
      graph,
      [&graph](subtree_generator &subs) -> subtree_type {
        subtree_type max{graph};
        for (const auto &sub : subs) {
          if (sub.n_induced() > max.n_induced()) {
            max = sub;
          }
        }
        return max;
      },
      [&graph](const std::vector<subtree_type> &subs) -> subtree_type {
        subtree_type max{graph};
        for (const auto &sub : subs) {
          if (sub.n_induced() > max.n_induced()) {
            max = sub;
          }
        }
        return max;
      });

  std::cout << max_subtree.n_induced() << '\n';
  std::cout << detail::dim_subtree{dims, max_subtree} << '\n';
}
