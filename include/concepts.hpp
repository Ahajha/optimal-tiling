#pragma once

#include <concepts>
#include <range/v3/all.hpp>

namespace detail {
template <class Range, class Item>
concept range_of =
    ranges::range<Range> && std::same_as<ranges::range_value_t<Range>, Item>;

template <class Range, class Item>
concept range_of_convertible_to = ranges::range<Range> &&
    std::convertible_to<ranges::range_value_t<Range>, Item>;
} // namespace detail