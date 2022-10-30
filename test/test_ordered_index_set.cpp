#include "ordered_index_set.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <array>
#include <concepts>

TEMPLATE_TEST_CASE("Ordered index set", "Ordered index set",
                   ordered_index_set<int>, (ordered_index_set<int, 20>)) {
  auto set = [] {
    if constexpr (std::same_as<TestType, ordered_index_set<int>>) {
      return TestType(20);
    } else {
      return TestType{};
    }
  }();

  CHECK(set.empty());
  CHECK(set.size() == 0);
  CHECK(std::ranges::equal(set, std::array<int, 0>{}));

  SECTION("Push front") {
    set.push_front(5);
    CHECK(set.size() == 1);
    CHECK(std::ranges::equal(set, std::array{5}));

    set.push_front(10);
    CHECK(set.size() == 2);
    CHECK(std::ranges::equal(set, std::array{10, 5}));

    set.push_front(15);
    CHECK(set.size() == 3);
    CHECK(std::ranges::equal(set, std::array{15, 10, 5}));

    CHECK(set.contains(5));
    CHECK(set.contains(10));
    CHECK(set.contains(15));
  }

  SECTION("Push back") {
    set.push_back(5);
    CHECK(set.size() == 1);
    CHECK(std::ranges::equal(set, std::array{5}));

    set.push_back(10);
    CHECK(set.size() == 2);
    CHECK(std::ranges::equal(set, std::array{5, 10}));

    set.push_back(15);
    CHECK(set.size() == 3);
    CHECK(std::ranges::equal(set, std::array{5, 10, 15}));

    CHECK(set.contains(5));
    CHECK(set.contains(10));
    CHECK(set.contains(15));
  }

  SECTION("Remove") {
    SECTION("Size == 1") {
      set.push_back(5);

      CHECK(set.contains(5));

      SECTION("Remove with one element") {
        CHECK(set.remove(5));
        CHECK(set.size() == 0);
        CHECK(std::ranges::equal(set, std::array<int, 0>{}));

        CHECK(!set.contains(5));
        CHECK(!set.remove(5));
        CHECK(!set.contains(5));
      }
    }

    SECTION("Size == 2") {
      set.push_back(5);
      set.push_back(10);

      CHECK(set.contains(5));
      CHECK(set.contains(10));

      SECTION("Remove first of two elements") {
        CHECK(set.remove(5));
        CHECK(set.size() == 1);
        CHECK(std::ranges::equal(set, std::array{10}));

        CHECK(set.contains(10));
        CHECK(!set.contains(5));
        CHECK(!set.remove(5));
        CHECK(!set.contains(5));
      }

      SECTION("Remove second of two elements") {
        CHECK(set.remove(10));
        CHECK(set.size() == 1);
        CHECK(std::ranges::equal(set, std::array{5}));

        CHECK(set.contains(5));
        CHECK(!set.contains(10));
        CHECK(!set.remove(10));
        CHECK(!set.contains(10));
      }
    }

    SECTION("Size == 3") {
      set.push_back(5);
      set.push_back(10);
      set.push_back(15);

      CHECK(set.contains(5));
      CHECK(set.contains(10));
      CHECK(set.contains(15));

      SECTION("Remove first of three elements") {
        CHECK(set.remove(5));
        CHECK(set.size() == 2);
        CHECK(std::ranges::equal(set, std::array{10, 15}));

        CHECK(set.contains(10));
        CHECK(set.contains(15));
        CHECK(!set.contains(5));
        CHECK(!set.remove(5));
        CHECK(!set.contains(5));
      }

      SECTION("Remove second of three elements") {
        CHECK(set.remove(10));
        CHECK(set.size() == 2);
        CHECK(std::ranges::equal(set, std::array{5, 15}));

        CHECK(set.contains(5));
        CHECK(set.contains(15));
        CHECK(!set.contains(10));
        CHECK(!set.remove(10));
        CHECK(!set.contains(10));
      }

      SECTION("Remove third of three elements") {
        CHECK(set.remove(15));
        CHECK(set.size() == 2);
        CHECK(std::ranges::equal(set, std::array{5, 10}));

        CHECK(set.contains(5));
        CHECK(set.contains(10));
        CHECK(!set.contains(15));
        CHECK(!set.remove(15));
        CHECK(!set.contains(15));
      }
    }
  }
  SECTION("Pop front/back") {
    SECTION("Size == 1") {
      set.push_back(5);

      CHECK(set.contains(5));

      SECTION("Pop front with one element") {
        CHECK(set.pop_front() == 5);
        CHECK(set.size() == 0);
        CHECK(std::ranges::equal(set, std::array<int, 0>{}));

        CHECK(!set.contains(5));
      }

      SECTION("Pop back with one element") {
        CHECK(set.pop_back() == 5);
        CHECK(set.size() == 0);
        CHECK(std::ranges::equal(set, std::array<int, 0>{}));

        CHECK(!set.contains(5));
      }
    }

    SECTION("Size == 2") {
      set.push_back(5);
      set.push_back(10);

      CHECK(set.contains(5));
      CHECK(set.contains(10));

      SECTION("Pop front with two elements") {
        CHECK(set.pop_front() == 5);
        CHECK(set.size() == 1);
        CHECK(std::ranges::equal(set, std::array{10}));

        CHECK(!set.contains(5));
        CHECK(set.contains(10));
      }

      SECTION("Pop back with two elements") {
        CHECK(set.pop_back() == 10);
        CHECK(set.size() == 1);
        CHECK(std::ranges::equal(set, std::array{5}));

        CHECK(set.contains(5));
        CHECK(!set.contains(10));
      }
    }

    SECTION("Size == 3") {
      set.push_back(5);
      set.push_back(10);
      set.push_back(15);

      CHECK(set.contains(5));
      CHECK(set.contains(10));
      CHECK(set.contains(15));

      SECTION("Pop front with three elements") {
        CHECK(set.pop_front() == 5);
        CHECK(set.size() == 2);
        CHECK(std::ranges::equal(set, std::array{10, 15}));

        CHECK(!set.contains(5));
        CHECK(set.contains(10));
        CHECK(set.contains(15));
      }

      SECTION("Pop back with three elements") {
        CHECK(set.pop_back() == 15);
        CHECK(set.size() == 2);
        CHECK(std::ranges::equal(set, std::array{5, 10}));

        CHECK(set.contains(5));
        CHECK(set.contains(10));
        CHECK(!set.contains(15));
      }
    }
  }
}
