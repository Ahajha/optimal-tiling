#pragma once

#include "config.hpp"
#include "graph.hpp"
#include "subtree_test_helpers.hpp"

#include <cppcoro/recursive_generator.hpp>

#include <vector>

namespace testing {
/**
 * @brief Provides reference implementation of induced subtree enumeration.
 * @param dimensions The dimensions of the graph to
 * @return std::vector<subtree_snapshot>
 */
cppcoro::recursive_generator<subtree_type>
brute_force_enumerate(const graph_type &dimensions);

} // namespace testing