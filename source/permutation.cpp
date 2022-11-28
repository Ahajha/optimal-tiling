#include "permutation.hpp"

#include <hrp/src/lib.rs.h>

permutation_set::permutation_set(
    const std::span<const std::size_t> &dimensions) {
  const auto perms = hrp::get_permutation_set(
      rust::Slice(dimensions.data(), dimensions.size()));

  m_permutations.reserve(perms.size());
  for (const auto &perm : perms) {
    m_permutations.emplace_back(perm.value.begin(), perm.value.end());
  }
}

std::size_t permutation_set::n_permutations(
    const std::span<const std::size_t> &dimensions) {
  return hrp::n_permutations(rust::Slice(dimensions.data(), dimensions.size()));
}
