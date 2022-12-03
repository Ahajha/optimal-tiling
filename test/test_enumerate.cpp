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

TEST_CASE("Size = {2,2}, from permutations") {
  const graph_type graph{2, 2};
  const std::vector<std::size_t> dims{2, 2};
  const permutation_set perm_set{dims};

  using u8 = std::uint8_t;

  const subtree_set expected_subtree_bases{
      {graph, std::vector<u8>{}},
      {graph, std::vector<u8>{0}},
      {graph, std::vector<u8>{0, 1}},
      {graph, std::vector<u8>{0, 1, 2}},
  };

  subtree_set expected_subtrees;

  for (const auto &base : expected_subtree_bases) {
    for (const auto &perm : perm_set.perms()) {
      expected_subtrees.emplace(base.apply_permutation(perm));
    }
  }

  check_result(graph, expected_subtrees, enumerate);
}

TEST_CASE("Size = {3,3}, from permutations") {
  const graph_type graph{3, 3};
  const std::vector<std::size_t> dims{3, 3};
  const permutation_set perm_set{dims};

  using u8 = std::uint8_t;

  const subtree_set expected_subtree_bases{
      {graph, std::vector<u8>{}},
      {graph, std::vector<u8>{0}},
      {graph, std::vector<u8>{1}},
      {graph, std::vector<u8>{4}},
      {graph, std::vector<u8>{0, 1}},
      {graph, std::vector<u8>{1, 4}},
      {graph, std::vector<u8>{0, 1, 2}},
      {graph, std::vector<u8>{0, 1, 3}},
      {graph, std::vector<u8>{0, 1, 4}},
      {graph, std::vector<u8>{1, 3, 4}},
      {graph, std::vector<u8>{1, 4, 7}},
      {graph, std::vector<u8>{0, 1, 2, 3}},
      {graph, std::vector<u8>{0, 1, 2, 4}},
      {graph, std::vector<u8>{0, 1, 4, 5}},
      {graph, std::vector<u8>{0, 1, 4, 7}},
      {graph, std::vector<u8>{1, 3, 4, 5}},
      {graph, std::vector<u8>{0, 1, 2, 3, 5}},
      {graph, std::vector<u8>{0, 1, 2, 3, 6}},
      {graph, std::vector<u8>{0, 1, 2, 4, 7}},
      {graph, std::vector<u8>{0, 1, 4, 5, 7}},
      {graph, std::vector<u8>{0, 1, 4, 5, 8}},
      {graph, std::vector<u8>{0, 1, 4, 7, 8}},
      {graph, std::vector<u8>{0, 2, 3, 4, 5}},
      {graph, std::vector<u8>{1, 3, 4, 5, 7}},
      {graph, std::vector<u8>{0, 1, 2, 3, 6, 7}},
      {graph, std::vector<u8>{0, 1, 2, 4, 6, 7}},
      {graph, std::vector<u8>{0, 1, 4, 5, 6, 7}},
      {graph, std::vector<u8>{0, 1, 2, 3, 5, 6, 7}},
      {graph, std::vector<u8>{0, 1, 2, 3, 5, 6, 8}},
      {graph, std::vector<u8>{0, 1, 2, 4, 6, 7, 8}},
  };

  subtree_set expected_subtrees;

  for (const auto &base : expected_subtree_bases) {
    for (const auto &perm : perm_set.perms()) {
      expected_subtrees.emplace(base.apply_permutation(perm));
    }
  }

  check_result(graph, expected_subtrees, enumerate);
}