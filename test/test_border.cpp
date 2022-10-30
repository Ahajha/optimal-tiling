#include "border.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Border") {
  SECTION("Size = {1}") {
    graph_type graph{1};
    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    subtree_type sub{graph, 0};
    update(sub, border, 0, history);

    CHECK(std::ranges::equal(border, std::array<vertex_id, 0>{}));
    CHECK(history.size() == 1);
  }

  SECTION("Size = {2}") {
    graph_type graph{2};
    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    SECTION("Root == 0") {
      subtree_type sub{graph, 0};
      update(sub, border, 0, history);

      CHECK(std::ranges::equal(border, std::array{1}));
      CHECK(history.size() == 2);

      {
        CHECK(border.pop_front() == 1);
        CHECK(border.empty());
        sub.add(1);
        update(sub, border, 1, history);

        CHECK(border.empty());
        CHECK(history.size() == 3);

        restore(border, history);
        CHECK(border.empty());
        sub.rem(1);
      }

      restore(border, history);
      CHECK(std::ranges::equal(border, std::array{1}));
      CHECK(history.empty());
    }

    SECTION("Root == 1") {
      subtree_type sub{graph, 1};
      update(sub, border, 1, history);

      CHECK(border.empty());
      CHECK(history.size() == 1);

      restore(border, history);
      CHECK(border.empty());
      CHECK(history.empty());
    }
  }

  SECTION("Size = {3}") {
    graph_type graph{3};
    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    SECTION("Root == 0") {
      subtree_type sub{graph, 0};
      update(sub, border, 0, history);

      CHECK(std::ranges::equal(border, std::array{1}));
      CHECK(history.size() == 2);

      {
        CHECK(border.pop_front() == 1);
        CHECK(border.empty());
        sub.add(1);
        update(sub, border, 1, history);

        CHECK(std::ranges::equal(border, std::array{2}));
        CHECK(history.size() == 4);

        {
          CHECK(border.pop_front() == 2);
          CHECK(border.empty());
          sub.add(2);
          update(sub, border, 2, history);

          CHECK(border.empty());
          CHECK(history.size() == 5);

          restore(border, history);
          CHECK(border.empty());
          CHECK(history.size() == 4);
          sub.rem(2);
        }

        restore(border, history);
        CHECK(std::ranges::equal(border, std::array{2}));
        CHECK(history.size() == 2);
        sub.rem(1);
      }

      // WHAT??
      restore(border, history);
      CHECK(std::ranges::equal(border, std::array{1, 2}));
      CHECK(history.empty());
    }

    // SECTION("Root == 1") {
    //   subtree_type sub{graph, 1};
    //   update(sub, border, 1, history);
    //
    //  CHECK(border.empty());
    //  CHECK(history.size() == 1);
    //
    //  restore(border, history);
    //  CHECK(border.empty());
    //  CHECK(history.empty());
    //}
  }
}
