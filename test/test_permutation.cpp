#include "permutation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <range/v3/all.hpp>

#include <set>
#include <vector>

TEST_CASE("Permutations") {
  SECTION("Dims = {}") {
    std::vector<std::size_t> dims{};
    REQUIRE(permutation_set::n_permutations(dims) == 1);

    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 1);
    CHECK(perm_set.perms() == std::vector<std::vector<vertex_id>>{{0}});
  }

  SECTION("Dims = {1}") {
    std::vector<std::size_t> dims{1};
    REQUIRE(permutation_set::n_permutations(dims) == 1);

    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 1);
    CHECK(perm_set.perms() == std::vector<std::vector<vertex_id>>{{0}});
  }

  SECTION("Dims = {2}") {
    std::vector<std::size_t> dims{2};
    REQUIRE(permutation_set::n_permutations(dims) == 2);
    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 2);
    CHECK(perm_set.perms() ==
          std::vector<std::vector<vertex_id>>{{0, 1}, {1, 0}});
  }

  SECTION("Dims = {2, 2}") {
    std::vector<std::size_t> dims{2, 2};
    REQUIRE(permutation_set::n_permutations(dims) == 8);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 8);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 8);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{
                          {0, 1, 2, 3},
                          {0, 2, 1, 3},
                          {1, 0, 3, 2},
                          {2, 0, 3, 1},
                          {1, 3, 0, 2},
                          {2, 3, 0, 1},
                          {3, 1, 2, 0},
                          {3, 2, 1, 0},
                      });
  }

  SECTION("Dims = {3, 2}") {
    std::vector<std::size_t> dims{3, 2};
    REQUIRE(permutation_set::n_permutations(dims) == 4);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 4);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 4);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{
                          {0, 1, 2, 3, 4, 5},
                          {3, 4, 5, 0, 1, 2},
                          {2, 1, 0, 5, 4, 3},
                          {5, 4, 3, 2, 1, 0},
                      });
  }

  SECTION("Dims = {2, 3}") {
    std::vector<std::size_t> dims{2, 3};
    REQUIRE(permutation_set::n_permutations(dims) == 4);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 4);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 4);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{
                          {0, 1, 2, 3, 4, 5},
                          {4, 5, 2, 3, 0, 1},
                          {1, 0, 3, 2, 5, 4},
                          {5, 4, 3, 2, 1, 0},
                      });
  }

  SECTION("Dims = {2, 2, 2}") {
    std::vector<std::size_t> dims{2, 2, 2};
    REQUIRE(permutation_set::n_permutations(dims) == 48);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 48);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 48);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{
                          {0, 1, 2, 3, 4, 5, 6, 7}, // Base
                          {0, 1, 4, 5, 2, 3, 6, 7}, //
                          {0, 2, 1, 3, 4, 6, 5, 7}, //
                          {0, 2, 4, 6, 1, 3, 5, 7}, //
                          {0, 4, 1, 5, 2, 6, 3, 7}, //
                          {0, 4, 2, 6, 1, 5, 3, 7}, //
                          {1, 0, 3, 2, 5, 4, 7, 6}, // Reverse dim 0
                          {1, 0, 5, 4, 3, 2, 7, 6}, //
                          {1, 3, 0, 2, 5, 7, 4, 6}, //
                          {1, 3, 5, 7, 0, 2, 4, 6}, //
                          {1, 5, 0, 4, 3, 7, 2, 6}, //
                          {1, 5, 3, 7, 0, 4, 2, 6}, //
                          {2, 0, 3, 1, 6, 4, 7, 5}, //
                          {2, 0, 6, 4, 3, 1, 7, 5}, //
                          {2, 3, 0, 1, 6, 7, 4, 5}, // Reverse dim 1
                          {2, 3, 6, 7, 0, 1, 4, 5}, //
                          {2, 6, 0, 4, 3, 7, 1, 5}, //
                          {2, 6, 3, 7, 0, 4, 1, 5}, //
                          {3, 1, 2, 0, 7, 5, 6, 4}, //
                          {3, 1, 7, 5, 2, 0, 6, 4}, //
                          {3, 2, 1, 0, 7, 6, 5, 4}, // Reverse dim 0, 1
                          {3, 2, 7, 6, 1, 0, 5, 4}, //
                          {3, 7, 1, 5, 2, 6, 0, 4}, //
                          {3, 7, 2, 6, 1, 5, 0, 4}, //
                          {4, 0, 5, 1, 6, 2, 7, 3}, //
                          {4, 0, 6, 2, 5, 1, 7, 3}, //
                          {4, 5, 0, 1, 6, 7, 2, 3}, //
                          {4, 5, 6, 7, 0, 1, 2, 3}, // Reverse dim 2
                          {4, 6, 0, 2, 5, 7, 1, 3}, //
                          {4, 6, 5, 7, 0, 2, 1, 3}, //
                          {5, 1, 4, 0, 7, 3, 6, 2}, //
                          {5, 1, 7, 3, 4, 0, 6, 2}, //
                          {5, 4, 1, 0, 7, 6, 3, 2}, //
                          {5, 4, 7, 6, 1, 0, 3, 2}, // Reverse dim 0, 1
                          {5, 7, 1, 3, 4, 6, 0, 2}, //
                          {5, 7, 4, 6, 1, 3, 0, 2}, //
                          {6, 2, 4, 0, 7, 3, 5, 1}, //
                          {6, 2, 7, 3, 4, 0, 5, 1}, //
                          {6, 4, 2, 0, 7, 5, 3, 1}, //
                          {6, 4, 7, 5, 2, 0, 3, 1}, //
                          {6, 7, 2, 3, 4, 5, 0, 1}, //
                          {6, 7, 4, 5, 2, 3, 0, 1}, // Reverse dim 0, 2
                          {7, 3, 5, 1, 6, 2, 4, 0}, //
                          {7, 3, 6, 2, 5, 1, 4, 0}, //
                          {7, 5, 3, 1, 6, 4, 2, 0}, //
                          {7, 5, 6, 4, 3, 1, 2, 0}, //
                          {7, 6, 3, 2, 5, 4, 1, 0}, //
                          {7, 6, 5, 4, 3, 2, 1, 0}, // Reverse dim 0, 1, 2
                      });
  }

  SECTION("Dims = {3, 2, 2}") {
    std::vector<std::size_t> dims{3, 2, 2};
    REQUIRE(permutation_set::n_permutations(dims) == 16);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 16);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 16);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{{
                          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
                          {0, 1, 2, 6, 7, 8, 3, 4, 5, 9, 10, 11},
                          {2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9},
                          {2, 1, 0, 8, 7, 6, 5, 4, 3, 11, 10, 9},
                          {3, 4, 5, 0, 1, 2, 9, 10, 11, 6, 7, 8},
                          {3, 4, 5, 9, 10, 11, 0, 1, 2, 6, 7, 8},
                          {5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6},
                          {5, 4, 3, 11, 10, 9, 2, 1, 0, 8, 7, 6},
                          {6, 7, 8, 0, 1, 2, 9, 10, 11, 3, 4, 5},
                          {6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5},
                          {8, 7, 6, 2, 1, 0, 11, 10, 9, 5, 4, 3},
                          {8, 7, 6, 11, 10, 9, 2, 1, 0, 5, 4, 3},
                          {9, 10, 11, 3, 4, 5, 6, 7, 8, 0, 1, 2},
                          {9, 10, 11, 6, 7, 8, 3, 4, 5, 0, 1, 2},
                          {11, 10, 9, 5, 4, 3, 8, 7, 6, 2, 1, 0},
                          {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                      }});
  }

  SECTION("Dims = {2, 3, 2}") {
    std::vector<std::size_t> dims{2, 3, 2};
    REQUIRE(permutation_set::n_permutations(dims) == 16);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 16);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 16);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{{
                          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
                          {0, 6, 2, 8, 4, 10, 1, 7, 3, 9, 5, 11},
                          {1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10},
                          {1, 7, 3, 9, 5, 11, 0, 6, 2, 8, 4, 10},
                          {4, 5, 2, 3, 0, 1, 10, 11, 8, 9, 6, 7},
                          {4, 10, 2, 8, 0, 6, 5, 11, 3, 9, 1, 7},
                          {5, 4, 3, 2, 1, 0, 11, 10, 9, 8, 7, 6},
                          {5, 11, 3, 9, 1, 7, 4, 10, 2, 8, 0, 6},
                          {6, 0, 8, 2, 10, 4, 7, 1, 9, 3, 11, 5},
                          {6, 7, 8, 9, 10, 11, 0, 1, 2, 3, 4, 5},
                          {7, 1, 9, 3, 11, 5, 6, 0, 8, 2, 10, 4},
                          {7, 6, 9, 8, 11, 10, 1, 0, 3, 2, 5, 4},
                          {10, 4, 8, 2, 6, 0, 11, 5, 9, 3, 7, 1},
                          {10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1},
                          {11, 5, 9, 3, 7, 1, 10, 4, 8, 2, 6, 0},
                          {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                      }});
  }

  SECTION("Dims = {2, 2, 3}") {
    std::vector<std::size_t> dims{2, 2, 3};
    REQUIRE(permutation_set::n_permutations(dims) == 16);

    const auto perm_vec = permutation_set(dims);
    REQUIRE(perm_vec.perms().size() == 16);

    const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
    REQUIRE(perm_set.size() == 16);

    CHECK(perm_set == std::set<std::vector<vertex_id>>{{
                          {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
                          {0, 2, 1, 3, 4, 6, 5, 7, 8, 10, 9, 11},
                          {1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10},
                          {1, 3, 0, 2, 5, 7, 4, 6, 9, 11, 8, 10},
                          {2, 0, 3, 1, 6, 4, 7, 5, 10, 8, 11, 9},
                          {2, 3, 0, 1, 6, 7, 4, 5, 10, 11, 8, 9},
                          {3, 1, 2, 0, 7, 5, 6, 4, 11, 9, 10, 8},
                          {3, 2, 1, 0, 7, 6, 5, 4, 11, 10, 9, 8},
                          {8, 9, 10, 11, 4, 5, 6, 7, 0, 1, 2, 3},
                          {8, 10, 9, 11, 4, 6, 5, 7, 0, 2, 1, 3},
                          {9, 8, 11, 10, 5, 4, 7, 6, 1, 0, 3, 2},
                          {9, 11, 8, 10, 5, 7, 4, 6, 1, 3, 0, 2},
                          {10, 8, 11, 9, 6, 4, 7, 5, 2, 0, 3, 1},
                          {10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1},
                          {11, 9, 10, 8, 7, 5, 6, 4, 3, 1, 2, 0},
                          {11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
                      }});
  }

  SECTION("Large dims") {
    // This doesn't enumerate the permutations manually, but just checks a few
    // basic invariants of some larger, more complicated dimension sets.

    // .first = dims, .second = expected number of permutations
    std::vector<std::pair<std::vector<std::size_t>, std::size_t>> dim_lists{
        {{4, 3, 4, 3}, 16 * 2 * 2}, {{3, 3, 3}, 8 * 6},
        {{2, 2, 2, 2}, 16 * 24},    {{2, 2, 2, 2, 2}, 32 * 120},
        {{2, 2, 2, 3}, 16 * 6},     {{3, 3, 2, 3, 3}, 32 * 24},
        {{1, 1, 1, 1, 1}, 1},       {{1, 2, 1, 2, 1}, 4 * 2},
    };

    for (const auto &[dims, expected] : dim_lists) {
      REQUIRE(permutation_set::n_permutations(dims) == expected);

      const auto n_vertices = ranges::accumulate(dims, 1ull, std::multiplies{});

      const auto perm_vec = permutation_set(dims);
      REQUIRE(perm_vec.perms().size() == expected);

      const std::set perm_set(perm_vec.perms().begin(), perm_vec.perms().end());
      REQUIRE(perm_set.size() == expected);

      // Test that all elements are accounted for
      for (const auto &perm : perm_vec.perms()) {
        CHECK(perm.size() == n_vertices);
        auto perm_copy = perm;
        ranges::sort(perm_copy);
        CHECK(ranges::equal(perm_copy, ranges::views::iota(0ull, n_vertices)));
      }
    }
  }
}
