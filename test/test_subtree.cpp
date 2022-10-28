#include "subtree.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

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

template <class graph_t>
std::ostream &operator<<(std::ostream &stream, const subtree<graph_t> &sub) {
  stream << "n_induced = " << static_cast<int>(sub.n_induced()) << '\n';
  stream << "cells:\n" << std::boolalpha;
  for (std::size_t i = 0; i < sub.base().vertices.size(); ++i) {
    stream << "Vertex " << i
           << ", has = " << sub.has(static_cast<graph_t::vertex_id>(i))
           << ", count = "
           << static_cast<int>(sub.cnt(static_cast<graph_t::vertex_id>(i)))
           << '\n';
  }
  return stream;
}

std::ostream &operator<<(std::ostream &stream, const subtree_snapshot &sub) {
  stream << "n_induced = " << sub.n_induced << '\n';
  stream << "cells:\n" << std::boolalpha;
  for (std::size_t i = 0; i < sub.cells.size(); ++i) {
    stream << "Vertex " << i << ", has = " << sub.cells[i].has
           << ", count = " << sub.cells[i].count << '\n';
  }
  return stream;
}

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

  SECTION("dimensions: {2,2}") {
    static_hrp_graph<2, 2> s_graph;
    hrp_graph r_graph{2, 2};

    subtree s_subtree(s_graph, 0);
    subtree r_subtree(r_graph, 0);

    const subtree_snapshot snapshot1{
        .cells =
            {
                {.count = 0, .has = true},
                {.count = 1, .has = false},
                {.count = 1, .has = false},
                {.count = 0, .has = false},
            },
        .n_induced = 1,
    };

    CHECK(snapshot1 == s_subtree);
    CHECK(snapshot1 == r_subtree);

    REQUIRE(s_subtree.add(1));
    REQUIRE(r_subtree.add(1));

    const subtree_snapshot snapshot2{
        .cells =
            {
                {.count = 1, .has = true},
                {.count = 1, .has = true},
                {.count = 1, .has = false},
                {.count = 1, .has = false},
            },
        .n_induced = 2,
    };

    CHECK(snapshot2 == s_subtree);
    CHECK(snapshot2 == r_subtree);

    REQUIRE(s_subtree.add(2));
    REQUIRE(r_subtree.add(2));

    const subtree_snapshot snapshot3{
        .cells =
            {
                {.count = 2, .has = true},
                {.count = 1, .has = true},
                {.count = 1, .has = true},
                {.count = 2, .has = false},
            },
        .n_induced = 3,
    };

    CHECK(snapshot3 == s_subtree);
    CHECK(snapshot3 == r_subtree);

    s_subtree.rem(2);
    r_subtree.rem(2);

    CHECK(snapshot2 == s_subtree);
    CHECK(snapshot2 == r_subtree);

    s_subtree.rem(1);
    r_subtree.rem(1);

    CHECK(snapshot1 == s_subtree);
    CHECK(snapshot1 == r_subtree);
  }

  SECTION("dimensions: {3,3,3}") {
    static_hrp_graph<3, 3, 3> s_graph;
    hrp_graph r_graph{3, 3, 3};

    /**
     * We will construct the densest induced subtree in a 3x3x3 graph, which
     * contains 18 vertices. One possible configuration looks like this, shown
     * layer by layer:
     *
     * X_X    X_X    XX_
     * XXX    _X_    X_X
     * X_X    X_X    _XX
     */

    // Start at the center vertex, just for fun.
    subtree s_subtree(s_graph, 13);
    subtree r_subtree(r_graph, 13);

    const subtree_snapshot snapshot1{
        .cells =
            {
                {.count = 0, .has = false}, // 0
                {.count = 0, .has = false}, // 1
                {.count = 0, .has = false}, // 2
                {.count = 0, .has = false}, // 3
                {.count = 1, .has = false}, // 4 (down)
                {.count = 0, .has = false}, // 5
                {.count = 0, .has = false}, // 6
                {.count = 0, .has = false}, // 7
                {.count = 0, .has = false}, // 8
                {.count = 0, .has = false}, // 9
                {.count = 1, .has = false}, // 10 (side)
                {.count = 0, .has = false}, // 11
                {.count = 1, .has = false}, // 12 (side)
                {.count = 0, .has = true},  // 13
                {.count = 1, .has = false}, // 14 (side)
                {.count = 0, .has = false}, // 15
                {.count = 1, .has = false}, // 16 (side)
                {.count = 0, .has = false}, // 17
                {.count = 0, .has = false}, // 18
                {.count = 0, .has = false}, // 19
                {.count = 0, .has = false}, // 20
                {.count = 0, .has = false}, // 21
                {.count = 1, .has = false}, // 22 (up)
                {.count = 0, .has = false}, // 23
                {.count = 0, .has = false}, // 24
                {.count = 0, .has = false}, // 25
                {.count = 0, .has = false}, // 26
            },
        .n_induced = 1,
    };

    CHECK(snapshot1 == s_subtree);
    CHECK(snapshot1 == r_subtree);

    // Down (to 4)
    REQUIRE(s_subtree.add(4));
    REQUIRE(r_subtree.add(4));

    auto snapshot2 = snapshot1;
    snapshot2.n_induced = 2;
    snapshot2.cells[4].has = true;
    snapshot2.cells[1].count = 1;
    snapshot2.cells[3].count = 1;
    snapshot2.cells[5].count = 1;
    snapshot2.cells[7].count = 1;
    snapshot2.cells[13].count = 1;

    CHECK(snapshot2 == s_subtree);
    CHECK(snapshot2 == r_subtree);

    // Out to the sides (3 and 5)
    REQUIRE(s_subtree.add(3));
    REQUIRE(r_subtree.add(3));

    REQUIRE(s_subtree.add(5));
    REQUIRE(r_subtree.add(5));

    auto snapshot3 = snapshot2;
    snapshot3.n_induced = 4;
    snapshot3.cells[3].has = true;
    snapshot3.cells[5].has = true;
    snapshot3.cells[4].count = 3;
    snapshot3.cells[0].count = 1;
    snapshot3.cells[2].count = 1;
    snapshot3.cells[6].count = 1;
    snapshot3.cells[8].count = 1;
    snapshot3.cells[12].count = 2;
    snapshot3.cells[14].count = 2;

    CHECK(snapshot3 == s_subtree);
    CHECK(snapshot3 == r_subtree);

    // Add the 4 corners (0,2,6,8)
    REQUIRE(s_subtree.add(0));
    REQUIRE(r_subtree.add(0));

    REQUIRE(s_subtree.add(2));
    REQUIRE(r_subtree.add(2));

    REQUIRE(s_subtree.add(6));
    REQUIRE(r_subtree.add(6));

    REQUIRE(s_subtree.add(8));
    REQUIRE(r_subtree.add(8));

    auto snapshot4 = snapshot3;
    snapshot4.n_induced = 8;
    snapshot4.cells[0].has = true;
    snapshot4.cells[2].has = true;
    snapshot4.cells[6].has = true;
    snapshot4.cells[8].has = true;
    snapshot4.cells[1].count = 3;
    snapshot4.cells[3].count = 3;
    snapshot4.cells[5].count = 3;
    snapshot4.cells[7].count = 3;
    snapshot4.cells[9].count = 1;
    snapshot4.cells[11].count = 1;
    snapshot4.cells[15].count = 1;
    snapshot4.cells[17].count = 1;

    CHECK(snapshot4 == s_subtree);
    CHECK(snapshot4 == r_subtree);

    // Add the 4 corners above the previous (9,11,15,17)
    REQUIRE(s_subtree.add(9));
    REQUIRE(r_subtree.add(9));

    REQUIRE(s_subtree.add(11));
    REQUIRE(r_subtree.add(11));

    REQUIRE(s_subtree.add(15));
    REQUIRE(r_subtree.add(15));

    REQUIRE(s_subtree.add(17));
    REQUIRE(r_subtree.add(17));

    auto snapshot5 = snapshot4;
    snapshot5.n_induced = 12;
    snapshot5.cells[9].has = true;
    snapshot5.cells[11].has = true;
    snapshot5.cells[15].has = true;
    snapshot5.cells[17].has = true;

    snapshot5.cells[0].count = 2;
    snapshot5.cells[2].count = 2;
    snapshot5.cells[6].count = 2;
    snapshot5.cells[8].count = 2;

    snapshot5.cells[18].count = 1;
    snapshot5.cells[20].count = 1;
    snapshot5.cells[24].count = 1;
    snapshot5.cells[26].count = 1;

    snapshot5.cells[10].count = 3;
    snapshot5.cells[12].count = 4;
    snapshot5.cells[14].count = 4;
    snapshot5.cells[16].count = 3;

    CHECK(snapshot5 == s_subtree);
    CHECK(snapshot5 == r_subtree);

    // Add 2 opposite corners above the previous (20, 24)
    REQUIRE(s_subtree.add(20));
    REQUIRE(r_subtree.add(20));

    REQUIRE(s_subtree.add(24));
    REQUIRE(r_subtree.add(24));

    auto snapshot6 = snapshot5;
    snapshot6.n_induced = 14;
    snapshot6.cells[20].has = true;
    snapshot6.cells[24].has = true;

    snapshot6.cells[11].count = 2;
    snapshot6.cells[19].count = 1;
    snapshot6.cells[23].count = 1;

    snapshot6.cells[15].count = 2;
    snapshot6.cells[25].count = 1;
    snapshot6.cells[21].count = 1;

    CHECK(snapshot6 == s_subtree);
    CHECK(snapshot6 == r_subtree);

    // Add the last 4 edges next to the previous (19, 21, 23, 25)
    REQUIRE(s_subtree.add(19));
    REQUIRE(r_subtree.add(19));

    REQUIRE(s_subtree.add(21));
    REQUIRE(r_subtree.add(21));

    REQUIRE(s_subtree.add(23));
    REQUIRE(r_subtree.add(23));

    REQUIRE(s_subtree.add(25));
    REQUIRE(r_subtree.add(25));

    auto snapshot7 = snapshot6;
    snapshot7.n_induced = 18;
    snapshot7.cells[19].has = true;
    snapshot7.cells[21].has = true;
    snapshot7.cells[23].has = true;
    snapshot7.cells[25].has = true;

    snapshot7.cells[18].count = 3;
    snapshot7.cells[20].count = 3;
    snapshot7.cells[24].count = 3;
    snapshot7.cells[26].count = 3;

    snapshot7.cells[10].count = 4;
    snapshot7.cells[12].count = 5;
    snapshot7.cells[14].count = 5;
    snapshot7.cells[16].count = 4;

    snapshot7.cells[22].count = 5;

    CHECK(snapshot7 == s_subtree);
    CHECK(snapshot7 == r_subtree);
  }
}