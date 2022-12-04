#include "enumerate_subtrees.hpp"
#include "permutation.hpp"
#include "subtree_test_helpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

TEST_CASE("Size = {1}") {
  const graph_type graph{1};

  const subtree_snapshot_set expected_subtrees{
      {.cells = {{.count = 0, .has = false}}, .n_induced = 0},
      {.cells = {{.count = 0, .has = true}}, .n_induced = 1},
  };

  check_result(graph, expected_subtrees, enumerate);
}

TEST_CASE("Size = {2}") {
  const graph_type graph{2};

  const subtree_snapshot_set expected_subtrees{
      {.cells =
           {
               {.count = 0, .has = false},
               {.count = 0, .has = false},
           },
       .n_induced = 0},
      {.cells =
           {
               {.count = 0, .has = true},
               {.count = 1, .has = false},
           },
       .n_induced = 1},
      {.cells =
           {
               {.count = 1, .has = false},
               {.count = 0, .has = true},
           },
       .n_induced = 1},
      {.cells =
           {
               {.count = 1, .has = true},
               {.count = 1, .has = true},
           },
       .n_induced = 2},
  };

  check_result(graph, expected_subtrees, enumerate);
}

TEST_CASE("Size = {2,2}") {
  const graph_type graph{2, 2};

  const subtree_snapshot_set expected_subtrees{
      {.cells = {{.count = 0, .has = false},
                 {.count = 0, .has = false},
                 {.count = 0, .has = false},
                 {.count = 0, .has = false}},
       .n_induced = 0},
      {.cells = {{.count = 0, .has = true},
                 {.count = 1, .has = false},
                 {.count = 1, .has = false},
                 {.count = 0, .has = false}},
       .n_induced = 1},
      {.cells = {{.count = 1, .has = false},
                 {.count = 0, .has = true},
                 {.count = 0, .has = false},
                 {.count = 1, .has = false}},
       .n_induced = 1},
      {.cells = {{.count = 1, .has = false},
                 {.count = 0, .has = false},
                 {.count = 0, .has = true},
                 {.count = 1, .has = false}},
       .n_induced = 1},
      {.cells = {{.count = 0, .has = false},
                 {.count = 1, .has = false},
                 {.count = 1, .has = false},
                 {.count = 0, .has = true}},
       .n_induced = 1},
      {.cells = {{.count = 1, .has = true},
                 {.count = 1, .has = true},
                 {.count = 1, .has = false},
                 {.count = 1, .has = false}},
       .n_induced = 2},
      {.cells = {{.count = 1, .has = true},
                 {.count = 1, .has = false},
                 {.count = 1, .has = true},
                 {.count = 1, .has = false}},
       .n_induced = 2},
      {.cells = {{.count = 1, .has = false},
                 {.count = 1, .has = true},
                 {.count = 1, .has = false},
                 {.count = 1, .has = true}},
       .n_induced = 2},
      {.cells = {{.count = 1, .has = false},
                 {.count = 1, .has = false},
                 {.count = 1, .has = true},
                 {.count = 1, .has = true}},
       .n_induced = 2},
      {.cells = {{.count = 2, .has = true},
                 {.count = 1, .has = true},
                 {.count = 1, .has = true},
                 {.count = 2, .has = false}},
       .n_induced = 3},
      {.cells = {{.count = 1, .has = true},
                 {.count = 2, .has = true},
                 {.count = 2, .has = false},
                 {.count = 1, .has = true}},
       .n_induced = 3},
      {.cells = {{.count = 1, .has = true},
                 {.count = 2, .has = false},
                 {.count = 2, .has = true},
                 {.count = 1, .has = true}},
       .n_induced = 3},
      {.cells = {{.count = 2, .has = false},
                 {.count = 1, .has = true},
                 {.count = 1, .has = true},
                 {.count = 2, .has = true}},
       .n_induced = 3},
  };

  check_result(graph, expected_subtrees, enumerate);
}

using vertex_list_list = std::vector<std::vector<std::uint8_t>>;

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

  subtree_set result;
  for (const auto &sub : enumerate(graph)) {
    result.emplace(sub);
  }

  CHECK(expected.size() == result.size());

  std::vector<subtree_type> res_min_exp, exp_min_res;
  ranges::set_difference(result, expected, std::back_inserter(res_min_exp));
  ranges::set_difference(expected, result, std::back_inserter(res_min_exp));

  for (const auto &sub : res_min_exp) {
    UNSCOPED_INFO(sub);
  }
  CHECK(res_min_exp.empty());

  for (const auto &sub : exp_min_res) {
    UNSCOPED_INFO(sub);
  }
  CHECK(exp_min_res.empty());
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