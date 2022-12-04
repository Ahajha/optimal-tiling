#pragma once

#include "enumerate_subtrees.hpp"
#include "reference_enumerator.hpp"
#include "subtree.hpp"

#include <catch2/catch_test_macros.hpp>

#include <range/v3/all.hpp>
#include <range/v3/view/zip.hpp>

#include <iostream>
#include <ranges>
#include <set>
#include <vector>

struct subtree_snapshot {
  struct cell {
    std::uint32_t count;
    bool has;
  };

  std::vector<cell> cells;
  std::uint32_t n_induced;

  template <class graph_t> bool operator==(const subtree<graph_t> &sub) const {
    if (cells.size() != sub.base_verts().size())
      return false;
    if (n_induced != sub.n_induced())
      return false;
    for (std::size_t i = 0; i < cells.size(); ++i) {
      if (cells[i].has != sub.has(static_cast<graph_t::vertex_id>(i)))
        return false;
      if (cells[i].count != sub.cnt(static_cast<graph_t::vertex_id>(i)))
        return false;
    }
    return true;
  }

  friend std::ostream &operator<<(std::ostream &stream,
                                  const subtree_snapshot &sub) {
    stream << "n_induced = " << sub.n_induced << '\n';
    stream << "cells:\n" << std::boolalpha;
    for (std::size_t i = 0; i < sub.cells.size(); ++i) {
      stream << "Vertex " << i << ", has = " << sub.cells[i].has
             << ", count = " << sub.cells[i].count << '\n';
    }
    return stream;
  }
};

template <class graph_t>
std::ostream &operator<<(std::ostream &stream, const subtree<graph_t> &sub) {
  stream << "n_induced = " << static_cast<int>(sub.n_induced()) << '\n';
  stream << "cells:\n" << std::boolalpha;
  for (std::size_t i = 0; i < sub.base_verts().size(); ++i) {
    stream << "Vertex " << i
           << ", has = " << sub.has(static_cast<graph_t::vertex_id>(i))
           << ", count = "
           << static_cast<int>(sub.cnt(static_cast<graph_t::vertex_id>(i)))
           << '\n';
  }
  return stream;
}
