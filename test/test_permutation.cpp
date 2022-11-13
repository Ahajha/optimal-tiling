#include "permutation.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Permutations") {
  std::vector<std::size_t> dims{};
  const auto perm_set = permutation_set(dims);
  REQUIRE(perm_set.perms().size() == 1);
  // CHECK(perm_set.perms().front() == std::vector<std::size_t>{0});
}