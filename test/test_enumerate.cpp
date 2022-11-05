#include "enumerate_subtrees.hpp"
#include "subtree_test_helpers.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

TEST_CASE("Size = {1}") {
  graph_type graph{1};

  const subtree_snapshot_set expected_subtrees{
      {.cells =
           {
               {.count = 0, .has = false},
           },
       .n_induced = 0},
      {.cells =
           {
               {.count = 0, .has = true},
           },
       .n_induced = 1},
  };

  auto result = enumerate(graph);

  subtree_set<graph_type> actual_subtrees(result.begin(), result.end());

  check_result(actual_subtrees, expected_subtrees);
}
