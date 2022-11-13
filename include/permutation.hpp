#pragma once

#include "config.hpp"

#include <span>
#include <vector>

/**
 * @brief Constructs a set of permutations that can be applied to a HRP graph
 * without affecting its structure, important for finding similar subgraphs that
 * differ only in permutation.
 */
class permutation_set {
public:
  permutation_set(const std::span<const std::size_t> &dimensions);

  const auto &perms() const { return m_permutations; }

private:
  // TODO: MDspan + Single contiguous vector?
  std::vector<std::vector<vertex_id>> m_permutations;
};