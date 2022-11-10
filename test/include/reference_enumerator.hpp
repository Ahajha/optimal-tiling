#pragma once

#include "config.hpp"
#include "graph.hpp"
#include "subtree_test_helpers.hpp"

#include <vector>

namespace testing {
/**
 * @brief Provides reference implementation of induced subtree enumeration.
 * @param dimensions The dimensions of the graph to
 * @return std::vector<subtree_snapshot>
 */
std::vector<subtree_snapshot>
brute_force_enumerate(const graph_type &dimensions);

} // namespace testing