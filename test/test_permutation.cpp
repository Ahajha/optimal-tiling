#include "permutation.hpp"

#include <catch2/catch_test_macros.hpp>

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
}
