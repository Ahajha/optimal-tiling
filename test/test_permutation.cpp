#include "permutation.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Permutations") {
  SECTION("Dims = {}") {
    std::vector<std::size_t> dims{};
    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 1);
    CHECK(perm_set.perms() == std::vector<std::vector<vertex_id>>{{0}});
  }

  SECTION("Dims = {1}") {
    std::vector<std::size_t> dims{1};
    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 1);
    CHECK(perm_set.perms() == std::vector<std::vector<vertex_id>>{{0}});
  }

  SECTION("Dims = {2}") {
    std::vector<std::size_t> dims{2};
    const auto perm_set = permutation_set(dims);
    REQUIRE(perm_set.perms().size() == 2);
    CHECK(perm_set.perms() ==
          std::vector<std::vector<vertex_id>>{{0, 1}, {1, 0}});
  }
}