#include "permutation.hpp"

#include <range/v3/all.hpp>

permutation_set::permutation_set(
    const std::span<const std::size_t> &dimensions) {
  m_permutations.reserve(n_permutations(dimensions));

  // If all dimensions are 0, then there is a single vertex, which can only
  // be "exchanged" with itself.
  if (dimensions.empty()) {
    m_permutations.emplace_back(std::vector<vertex_id>{0});
    return;
  }

  const auto n_vertices = ranges::fold_left(dimensions, 1, std::multiplies{});

  // dimension_partial_products[i] = product(0..<i)(dimensions[i])
  std::vector<std::size_t> dimension_partial_products(dimensions.size());
  dimension_partial_products.front() = 1;
  for (const auto index : ranges::views::iota(1ull, dimensions.size())) {
    dimension_partial_products[index] =
        dimension_partial_products[index - 1] * dimensions[index - 1];
  }

  const auto prev_n_vertices = dimension_partial_products.back();
  const auto primary_dimension = dimensions.back();

  // Otherwise, get the permutation set when the primary dimension is omitted
  const auto sub_permutations =
      permutation_set(dimensions.first(dimensions.size() - 1));

  // Ones provide no additional vertices, and cause complications in the
  // algorithm. We can just omit them entirely.
  if (primary_dimension == 1) {
    m_permutations = sub_permutations.m_permutations;
    return;
  }

  // New permutations are constructed by extending lower dimension ones
  for (const auto &sub_perm : sub_permutations.perms()) {
    // Create two reference permutations, one where the primary dimension is
    // preserved, and one where it is flipped.
    auto &forwards = m_permutations.emplace_back(n_vertices);
    auto &backwards = m_permutations.emplace_back(n_vertices);

    // Construct an initial form that just adds the next dimension.
    // Effectively, stacks the previous dimensions `primary_dimension` times.
    for (std::size_t height = 0; height < primary_dimension; ++height) {
      for (std::size_t coord = 0; coord < prev_n_vertices; ++coord) {
        const auto index = height * prev_n_vertices + coord;

        forwards[index] = height * prev_n_vertices + sub_perm[coord];
        backwards[index] = (primary_dimension - height - 1) * prev_n_vertices +
                           sub_perm[coord];
      }
    }

    // Then, for each dimension with the same length as the primary dimension
    // (other than the primary dimension itself), create a new permutation that
    // swaps that dimension with the primary.
    // clang-format off
    auto equal_dimensions = dimensions
        | ranges::view::drop_last(1)
        | ranges::view::enumerate
        | ranges::view::filter([&primary_dimension](const auto &item) {
          return item.second == primary_dimension;
        })
        | ranges::views::transform(&ranges::get<0, std::pair<std::size_t, std::size_t>>)
        ;
    // clang-format on

    for (const auto index : equal_dimensions) {
      const auto indexed_dim_size = dimension_partial_products[index];
      const auto indexed_dim = dimensions[index];
      assert(dimensions[index] == primary_dimension);

      const auto create_swapped_perm = [&](const auto &base_perm) {
        // Start with a reference permutation
        auto &new_perm = m_permutations.emplace_back(base_perm);

        // For each index, 'remove' each dimension and then re-add them,
        // swapped.
        for (auto &value : new_perm) {
          const auto primary_dim_value = value / prev_n_vertices;
          const auto indexed_dim_value =
              (value / indexed_dim_size) % indexed_dim;

          value -= primary_dim_value * prev_n_vertices +
                   indexed_dim_value * indexed_dim_size;
          value += indexed_dim_value * prev_n_vertices +
                   primary_dim_value * indexed_dim_size;
        }
      };

      create_swapped_perm(forwards);
      create_swapped_perm(backwards);
    }
  }
}

std::size_t permutation_set::n_permutations(
    const std::span<const std::size_t> &dimensions) {

  std::map<std::size_t, std::size_t> dim_counts;

  for (const auto dim : dimensions) {
    ++dim_counts[dim];
  }

  // Naive implementation of factorial
  constexpr static auto factorial = [](std::size_t n) {
    std::size_t result = 1;
    for (std::size_t i = 2; i <= n; ++i) {
      result *= i;
    }
    return result;
  };

  // Formula is 2^n * k1! * k2! * ...
  // where k1, k2, etc. are the counts of each unique dimension, and n is the
  // number of dimensions of size != 1.
  const auto n_ones = ranges::count(dimensions, 1);
  std::size_t result = 1 << (dimensions.size() - n_ones);
  for (const auto [n, count] : dim_counts) {
    // Also ignore ones here
    if (n != 1) {
      result *= factorial(count);
    }
  }
  return result;
}
