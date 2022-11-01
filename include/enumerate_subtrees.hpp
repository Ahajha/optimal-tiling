#pragma once

#include "config.hpp"
#include "enumerate_subtrees.hpp"

#include <cppcoro/recursive_generator.hpp>

cppcoro::recursive_generator<subtree_type> enumerate(graph_type graph);