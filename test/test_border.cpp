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
    CHECK(history.size() == 0);
  }

  SECTION("Size = {2}") {
    graph_type graph{2};
    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    SECTION("Root == 0") {
      subtree_type sub{graph, 0};
      update(sub, border, 0, history);

      CHECK(std::ranges::equal(border, std::array{1}));
      CHECK(history.size() == 1);
    }

    SECTION("Root == 1") {
      subtree_type sub{graph, 1};
      update(sub, border, 1, history);

      CHECK(std::ranges::equal(border, std::array<vertex_id, 0>{}));
      CHECK(history.size() == 0);
    }
  }

  SECTION("Size = {3}") {
    graph_type graph{2};
    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    SECTION("Root == 0") {
      subtree_type sub{graph, 0};
      update(sub, border, 0, history);

      CHECK(std::ranges::equal(border, std::array{1}));
      CHECK(history.size() == 1);
    }

    SECTION("Root == 1") {
      subtree_type sub{graph, 1};
      update(sub, border, 1, history);

      CHECK(std::ranges::equal(border, std::array<vertex_id, 0>{}));
      CHECK(history.size() == 0);
    }
  }
}