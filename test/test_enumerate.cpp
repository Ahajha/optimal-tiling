#include "enumerate_subtrees.hpp"
#include "permutation.hpp"

#include <catch2/catch_test_macros.hpp>
#include <range/v3/all.hpp>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <set>
#include <vector>

using subtree_set = std::set<subtree<graph_type>>;
using vertex_list_list = std::vector<std::vector<std::uint8_t>>;

/**
 * @brief Prints a subtree to a stream
 * @param stream The stream to print to
 * @param sub The subtree to print
 * @return The stream used for printing, used for chaining
 */
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
 * @brief Compares two sets of subtrees. Succeeds if the two sets are equal,
 * otherwise logs the differences between the sets and fails.
 * @param result The computed result of some algorithm
 * @param expected The expected result
 */
void compare_subtree_sets(const subtree_set &result,
                          const subtree_set &expected) {
  CHECK(expected.size() == result.size());

  std::vector<subtree_type> res_min_exp, exp_min_res;
  ranges::set_difference(result, expected, std::back_inserter(res_min_exp));
  ranges::set_difference(expected, result, std::back_inserter(exp_min_res));

  UNSCOPED_INFO("Extra subtrees in the result:");
  for (const auto &sub : res_min_exp) {
    UNSCOPED_INFO(sub);
  }
  CHECK(res_min_exp.empty());

  UNSCOPED_INFO("More subtrees expected:");
  for (const auto &sub : exp_min_res) {
    UNSCOPED_INFO(sub);
  }
  CHECK(exp_min_res.empty());
}

/**
 * @brief Runs all implemented enumeration algorithms on a graph and checks
 * their results against a set of expected subtrees.
 * @param graph The graph to enumerate subtrees of
 * @param expected The expected result
 */
void test_all_enumeration_algorithms(const graph_type &graph,
                                     const subtree_set &expected) {
  SECTION("Single threaded enumeration algorithm") {
    subtree_set result;
    for (const auto &sub : enumerate(graph)) {
      result.emplace(sub);
    }

    compare_subtree_sets(result, expected);
  }
}

/**
 * @brief Runs all enumeration tests on a given sized graph, given the expected
 * output.
 * @param dims The dimensions of the graph
 * @param expected_subtrees A list of vertex lists of each expected subtree.
 */
void check_exact_result(const std::span<const std::size_t> dims,
                        const vertex_list_list &expected_subtrees) {
  const graph_type graph{dims};

  subtree_set expected;
  for (const auto &base : expected_subtrees) {
    expected.emplace(graph, base);
  }

  test_all_enumeration_algorithms(graph, expected);
}

/**
 * @brief Runs all enumeration tests on a given sized graph, given the expected
 * output.
 * @param dims The dimensions of the graph
 * @param expected_subtree_bases A list of vertex lists of each expected
 * subtree, pruned of permutations.
 */
void check_result(const std::span<const std::size_t> dims,
                  const vertex_list_list &expected_subtree_bases) {
  const graph_type graph{dims};
  const permutation_set perm_set{dims};

  subtree_set expected;

  for (const auto &base : expected_subtree_bases) {
    const subtree_type base_sub{graph, base};
    for (const auto &perm : perm_set.perms()) {
      expected.emplace(base_sub.apply_permutation(perm));
    }
  }

  test_all_enumeration_algorithms(graph, expected);
}

TEST_CASE("Size = {1}") {
  const std::vector<std::size_t> dims{1};

  const vertex_list_list expected_subtrees{
      {},
      {0},
  };

  check_exact_result(dims, expected_subtrees);
}

TEST_CASE("Size = {2}") {
  const std::vector<std::size_t> dims{2};

  const vertex_list_list expected_subtrees{
      {},
      {0},
      {1},
      {0, 1},
  };

  check_exact_result(dims, expected_subtrees);
}

TEST_CASE("Size = {2,2}") {
  const std::vector<std::size_t> dims{2, 2};

  // clang-format off
  const vertex_list_list expected_subtrees{
      {},
      {0},
      {1},
      {2},
      {3},
      {0, 1},
      {0, 2},
      {1, 3},
      {2, 3},
      {0, 1, 2},
      {0, 1, 3},
      {0, 2, 3},
      {1, 2, 3},
  };
  // clang-format on

  check_exact_result(dims, expected_subtrees);
}

TEST_CASE("Size = {2,2}, from permutations") {
  const std::vector<std::size_t> dims{2, 2};

  const vertex_list_list expected_subtree_bases{
      {},
      {0},
      {0, 1},
      {0, 1, 2},
  };

  check_result(dims, expected_subtree_bases);
}

TEST_CASE("Size = {2,3}, from permutations") {
  const std::vector<std::size_t> dims{2, 3};

  const vertex_list_list expected_subtree_bases{
      {},
      {0},
      {2},
      {0, 1},
      {0, 2},
      {2, 3},
      {0, 1, 2},
      {0, 2, 3},
      {0, 2, 4},
      {0, 1, 2, 4},
      {0, 2, 3, 4},
      {0, 2, 3, 5},
      {0, 1, 2, 4, 5},
  };

  check_result(dims, expected_subtree_bases);
}

TEST_CASE("Size = {3,3}, from permutations") {
  const std::vector<std::size_t> dims{3, 3};

  const vertex_list_list expected_subtree_bases{
      {},
      {0},
      {1},
      {4},
      {0, 1},
      {1, 4},
      {0, 1, 2},
      {0, 1, 3},
      {0, 1, 4},
      {1, 3, 4},
      {1, 4, 7},
      {0, 1, 2, 3},
      {0, 1, 2, 4},
      {0, 1, 4, 5},
      {0, 1, 4, 7},
      {1, 3, 4, 5},
      {0, 1, 2, 3, 5},
      {0, 1, 2, 3, 6},
      {0, 1, 2, 4, 7},
      {0, 1, 4, 5, 7},
      {0, 1, 4, 5, 8},
      {0, 1, 4, 7, 8},
      {0, 2, 3, 4, 5},
      {1, 3, 4, 5, 7},
      {0, 1, 2, 3, 6, 7},
      {0, 1, 2, 4, 6, 7},
      {0, 1, 4, 5, 6, 7},
      {0, 1, 2, 3, 5, 6, 7},
      {0, 1, 2, 3, 5, 6, 8},
      {0, 1, 2, 4, 6, 7, 8},
  };

  check_result(dims, expected_subtree_bases);
}