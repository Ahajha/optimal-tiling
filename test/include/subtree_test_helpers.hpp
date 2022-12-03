#pragma once

#include "enumerate_subtrees.hpp"
#include "reference_enumerator.hpp"
#include "subtree.hpp"

#include <catch2/catch_test_macros.hpp>

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
    if (cells.size() != sub.base().vertices.size())
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
  for (std::size_t i = 0; i < sub.base().vertices.size(); ++i) {
    stream << "Vertex " << i
           << ", has = " << sub.has(static_cast<graph_t::vertex_id>(i))
           << ", count = "
           << static_cast<int>(sub.cnt(static_cast<graph_t::vertex_id>(i)))
           << '\n';
  }
  return stream;
}

/**
 * @brief Provides a less-than comparison for subtrees.
 */
struct subtree_compare {
  template <class graph_t>
  bool operator()(const subtree<graph_t> &sub1,
                  const subtree<graph_t> &sub2) const {
    if (sub1.n_induced() != sub2.n_induced()) {
      return sub1.n_induced() < sub2.n_induced();
    }
    for (typename graph_t::vertex_id i = 0; i < sub1.n_induced(); ++i) {
      if (sub1.has(i) != sub2.has(i)) {
        return sub1.has(i) < sub2.has(i);
      }
      if (sub1.cnt(i) != sub2.cnt(i)) {
        return sub1.cnt(i) < sub2.cnt(i);
      }
    }
    return true;
  }
};

/**
 * @brief Provides a less-than comparison for subtree snapshots.
 * Intended to be consistent with subtree_compare.
 */
struct subtree_snapshot_compare {
  bool operator()(const subtree_snapshot &sub1,
                  const subtree_snapshot &sub2) const {
    if (sub1.n_induced != sub2.n_induced) {
      return sub1.n_induced < sub2.n_induced;
    }
    for (const auto &[cell1, cell2] :
         ranges::views::zip(sub1.cells, sub2.cells)) {
      if (cell1.has != cell2.has) {
        return cell1.has < cell2.has;
      }
      if (cell1.count != cell2.count) {
        return cell1.count < cell2.count;
      }
    }
    return true;
  }
};

using subtree_snapshot_set =
    std::set<subtree_snapshot, subtree_snapshot_compare>;

using subtree_set = std::set<subtree<graph_type>>;

template <class graph_t>
void check_result(const graph_t &graph, const subtree_snapshot_set &expected,
                  const auto &&enumeration_impl) {
  auto result = enumeration_impl(graph);

  std::vector<subtree<graph_t>> result_vec(result.begin(), result.end());

  // It is possible that it produces duplicates, which a set will not catch, so
  // make sure the vector's size is the same as the expected set.
  REQUIRE(result_vec.size() == expected.size());

  std::set<subtree<graph_t>, subtree_compare> result_set(result_vec.begin(),
                                                         result_vec.end());

  for (const auto &[sub, snap] : ranges::views::zip(result_set, expected)) {
    CHECK(sub == snap);
  }
}

template <class graph_t>
void check_result(const graph_t &graph, const subtree_set &expected,
                  const auto &&enumeration_impl) {
  auto result = enumeration_impl(graph);

  std::vector<subtree<graph_t>> result_vec(result.begin(), result.end());

  // It is possible that it produces duplicates, which a set will not catch, so
  // make sure the vector's size is the same as the expected set.
  CHECK(result_vec.size() == expected.size());

  // For ease of testing and validating results, if this fails it will print
  // both set differences.

  subtree_set result_set(result_vec.begin(), result_vec.end());

  std::vector<subtree<graph_t>> res_min_exp, exp_min_res;
  ranges::set_difference(result_set, expected, std::back_inserter(res_min_exp));
  ranges::set_difference(expected, result_set, std::back_inserter(res_min_exp));

  for (const auto &sub : res_min_exp) {
    UNSCOPED_INFO(sub);
  }
  CHECK(res_min_exp.empty());

  for (const auto &sub : exp_min_res) {
    UNSCOPED_INFO(sub);
  }
  CHECK(exp_min_res.empty());
}
