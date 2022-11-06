#pragma once

#include "graph.hpp"
#include "ordered_index_set.hpp"
#include "subtree.hpp"

#include <stack>

using graph_type = hrp_graph;
using vertex_id = graph_type::vertex_id;
using subtree_type = subtree<graph_type>;
using border_type = ordered_index_set<vertex_id>;

enum class action_type { add, rem, stop };
struct action {
  action_type type;
  vertex_id id;
};

using history_type = std::stack<action, std::vector<action>>;
