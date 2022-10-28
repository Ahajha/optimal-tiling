#include "subtree.hpp"

#include <catch2/catch_test_macros.hpp>

struct subtree_snapshot {
  struct cell {
    std::uint32_t count;
    bool has;
  };

  std::vector<cell> cells;
  std::uint32_t n_induced;

  template <class graph_t> bool operator==(const subtree<graph_t> &sub) const {
    if (cells.size() != sub.base().vertices.size())
      return false;
    if (n_induced != sub.n_induced())
      return false;
    for (std::size_t i = 0; i < cells.size(); ++i) {
      if (cells[i].has != sub.has(static_cast<graph_t::vertex_id>(i)))
        return false;
      if (cells[i].count != sub.cnt(static_cast<graph_t::vertex_id>(i)))
        return false;
    }
    return true;
  }
};

TEST_CASE("Subtree") {
  SECTION("dimensions: {1}") {
    static_hrp_graph<1> s_graph;
    hrp_graph r_graph{1};

    subtree s_subtree(s_graph, 0);
    subtree r_subtree(r_graph, 0);

    const subtree_snapshot snapshot{
        .cells =
            {
                {.count = 0, .has = true},
            },
        .n_induced = 1,
    };

    CHECK(snapshot == s_subtree);
    CHECK(snapshot == r_subtree);
  }

  SECTION("dimensions: {2}") {
    static_hrp_graph<2> s_graph;
    hrp_graph r_graph{2};

    subtree s_subtree(s_graph, 0);
    subtree r_subtree(r_graph, 0);

    const subtree_snapshot snapshot1{
        .cells =
            {
                {.count = 0, .has = true},
                {.count = 1, .has = false},
            },
        .n_induced = 1,
    };

    CHECK(snapshot1 == s_subtree);
    CHECK(snapshot1 == r_subtree);

    s_subtree.add(1);
    r_subtree.add(1);

    const subtree_snapshot snapshot2{
        .cells =
            {
                {.count = 1, .has = true},
                {.count = 1, .has = true},
            },
        .n_induced = 2,
    };

    CHECK(snapshot2 == s_subtree);
    CHECK(snapshot2 == r_subtree);

    s_subtree.rem(1);
    r_subtree.rem(1);

    CHECK(snapshot1 == s_subtree);
    CHECK(snapshot1 == r_subtree);
  }
}