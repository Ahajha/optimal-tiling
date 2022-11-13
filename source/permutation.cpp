#include "permutation.hpp"

#include <range/v3/all.hpp>

permutation_set::permutation_set(
    const std::span<const std::size_t> &dimensions) {
  // If all dimensions are 0, then there is a single vertex, which can only
  // be "exchanged" with itself.
  if (dimensions.empty()) {
    m_permutations.emplace_back(std::vector<vertex_id>{0});
    return;
  }

  const auto n_vertices = ranges::fold_left(dimensions, 1, std::multiplies{});
  const auto prev_n_vertices = n_vertices / dimensions.back();

  // index i = the size of the graph after removing (i + 1) dimensions.
  std::vector<std::size_t> dimension_sizes(dimensions.size());
  dimension_sizes.back() = 1;
  for (const auto index :
       ranges::views::iota(1ull, dimensions.size()) | ranges::views::reverse) {
    dimension_sizes[index - 1] = dimension_sizes[index] * dimensions[index];
  }

  const auto primary_dimension = dimensions.back();

  // Otherwise, get the permutation set when the primary dimension is omitted
  const auto sub_permutations =
      permutation_set(dimensions.first(dimensions.size() - 1));

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
          return item.first == primary_dimension;
        })
        | ranges::views::transform(&ranges::get<1, std::pair<std::size_t, std::size_t>>)
        ;
    // clang-format on

    const auto primary_dim_size = dimension_sizes.front();
    for (const auto index : equal_dimensions) {
      const auto indexed_dim_size = dimension_sizes[index];
      const auto indexed_dim_height = dimensions[index];

      const auto create_swapped_perm = [&](const auto &base_perm) {
        // Start with a reference permutation
        auto &new_perm = m_permutations.emplace_back(base_perm);

        // For each index, 'remove' each dimension and then re-add them,
        // swapped.
        for (auto &value : new_perm) {
          const auto primary_dim_value = value / primary_dim_size;
          const auto indexed_dim_value =
              (value / indexed_dim_size) % indexed_dim_height;

          value -= primary_dim_value * primary_dim_size +
                   indexed_dim_value * indexed_dim_size;
          value += indexed_dim_value * primary_dim_size +
                   primary_dim_value * indexed_dim_size;
        }
      };

      create_swapped_perm(forwards);
      create_swapped_perm(backwards);
    }
  }
}