#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <limits>
#include <optional>
#include <ranges>
#include <vector>

/**
 * @brief An ordered index set is a subset of a fixed range of indexes where the
 * order of members matter. It features:
 * 1. Constant time lookup and removal
 * 2. Constant time addition at the ends (This implementation does not support
 * insertion in the middle, for simplicity)
 *
 * @tparam index_type The type of the indexes to use
 * @param static_size 0 if the size is dynamic, otherwise the size is fixed at
 * compile time.
 */
template <std::integral index_type, index_type static_size = 0>
class ordered_index_set {
  // The null index is used as a "null" when specifying next/prev fields.
  // It must be an unused index.
  constexpr static index_type null_index =
      std::numeric_limits<index_type>::max();
  static_assert(null_index != static_size);

  /**
   * @brief The members are stored in a doubly linked list. This structure is
   * one chain in the list.
   */
  struct node {
    // True iff the node at the current index is in the set
    bool is_member{false};
    // The index of the next node in the chain, or null_index if this is either
    // the last index or !is_member
    index_type next{null_index};
    // The index of the previous node in the chain, or null_index if this is
    // either the first index or !is_member
    index_type prev{null_index};
  };

  /**
   * @brief The underlying container type to store the nodes.
   */
  using container_type = std::conditional_t<static_size == 0, std::vector<node>,
                                            std::array<node, static_size>>;

public:
  /**
   * @brief Default construct an ordered index set to an empty state.
   * Note: This overload is available only if the size is known at compile time.
   */
  constexpr ordered_index_set() requires(static_size != 0) = default;

  /**
   * @brief Default construct an ordered index set to an empty state.
   * Note: Assumes that size == static_size.
   * Note: This overload is available to provide parity with the non-static size
   * constructor with the same parameters.
   */
  constexpr ordered_index_set([[maybe_unused]] index_type size) requires(
      static_size != 0) {
    assert(size == static_size);
  }

  /**
   * @brief Default construct an ordered index set to an empty state.
   * @param size One larger than the largest index that this set can hold,
   * equivalent to the maximum number of indexes that this set can hold.
   * Note: This overload is only available if the size is not known at compile
   * time.
   */
  constexpr ordered_index_set(index_type size) requires(static_size == 0)
      : m_list(static_cast<std::size_t>(size)) {}

  /**
   * @brief Removes a specified index from the set if it exists.
   * @return true if the index was removed,
   * @return false if the index did not exist.
   */
  constexpr bool remove(index_type idx) {
    if (!contains(idx))
      return false;

    m_list[idx].is_member = false;

    // Removals are 'lazy', they do not reset the cell that was removed.
    if (m_head == idx) {
      m_head = m_list[idx].next;
    } else {
      m_list[m_list[idx].prev].next = m_list[idx].next;
    }

    if (m_tail == idx) {
      m_tail = m_list[idx].prev;
    } else {
      m_list[m_list[idx].next].prev = m_list[idx].prev;
    }

    --m_size;

    return true;
  }

  /**
   * @brief Adds an index to the set at the front of the list.
   * Assumes the index is not already contained in the set.
   * @param idx the index to insert
   */
  constexpr void push_front(index_type idx) {
    assert(!contains(idx));

    m_list[idx] = {
        .is_member = true,
        .next = m_head,
        .prev = null_index,
    };

    if (m_head == null_index) {
      m_tail = idx;
    } else {
      m_list[m_head].prev = idx;
    }

    m_head = idx;

    ++m_size;
  }

  /**
   * @brief Adds an index to the set at the back of the list.
   * Assumes the index is not already contained in the set.
   * @param idx the index to insert
   */
  constexpr void push_back(index_type idx) {
    assert(!contains(idx));

    m_list[idx] = {
        .is_member = true,
        .next = null_index,
        .prev = m_tail,
    };

    if (m_tail == null_index) {
      m_head = idx;
    } else {
      m_list[m_tail].next = idx;
    }

    m_tail = idx;

    ++m_size;
  }

  /**
   * @brief Removes the first item from the list.
   * Assumes there is at least one item in the set.
   * @return index_type The first item
   */
  constexpr index_type pop_front() {
    assert(!empty());

    const auto idx = m_head;
    remove(m_head);
    return idx;
  }

  /**
   * @brief Removes the last item from the list.
   * Assumes there is at least one item in the set.
   * @return index_type The last item
   */
  constexpr index_type pop_back() {
    assert(!empty());

    const auto idx = m_tail;
    remove(m_tail);
    return idx;
  }

  /**
   * @brief Checks if the set is empty
   * @return true if the set is empty
   * @return false if the set contains items
   */
  [[nodiscard]] constexpr bool empty() const { return m_size == 0; }

  /**
   * @brief Checks if the set contains a specific index
   * @param idx The index to check
   * @return true if the set contains the index
   * @return false if the set does not contain the index
   */
  [[nodiscard]] constexpr bool contains(index_type idx) const {
    return m_list[idx].is_member;
  }

  /**
   * @brief Gets the number of indexes currently in the set.
   * @return The number of indexes in the set
   */
  [[nodiscard]] constexpr index_type size() const { return m_size; }

  friend class iterator;

  /**
   * @brief An iterator over the elements over an ordered index set.
   */
  class iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = index_type;

    /**
     * @brief Default construct an iterator. Only defined to satisfy
     * ranges::begin/ranges::end.
     */
    [[nodiscard]] constexpr iterator() = default;

    /**
     * @brief Constructs an ordered index set iterator
     * @param host_list The list to iterate over
     * @param start_index The first index to start iteration
     */
    [[nodiscard]] constexpr iterator(const container_type &host_list,
                                     index_type start_index)
        : m_current_index(start_index), m_host_list(&host_list) {}

    /**
     * @brief Advances this iterator to the next index in the list.
     * @return *this
     */
    constexpr iterator &operator++() {
      if (m_current_index != null_index)
        m_current_index = (*m_host_list)[m_current_index].next;
      return *this;
    }

    /**
     * @brief Advances this iterator to the next index in the list.
     * @return The original state of the iterator
     */
    [[nodiscard(
        "Use pre-incrementing if ignoring the result")]] constexpr iterator
    operator++(int) {
      auto it{*this};
      ++*this;
      return it;
    }

    /**
     * @brief Checks if two iterators are equal
     * @param i The other iterator to compare to
     * @return true if both iterators currently contain the same element
     * @return false if the elements in the iterators differ
     */
    [[nodiscard]] constexpr bool operator==(const iterator &i) const {
      return m_current_index == i.m_current_index;
    }

    /**
     * @brief Gets the index currently referenced by this iterator
     * @return the index currently referenced by this iterator
     */
    [[nodiscard]] constexpr index_type operator*() const {
      return m_current_index;
    }

  private:
    index_type m_current_index{};
    const container_type *m_host_list{};
  };

  static_assert(std::input_iterator<iterator>);

  /**
   * @brief Obtains an iterator to the beginning of this list.
   * @return an iterator to the beginning of this list.
   */
  [[nodiscard]] constexpr iterator begin() const { return {m_list, m_head}; }

  /**
   * @brief Obtains an iterator to the end of this list.
   * @return an iterator to the end of this list.
   */
  [[nodiscard]] constexpr iterator end() const { return {m_list, null_index}; }

private:
  container_type m_list{};
  index_type m_size{0};
  index_type m_head{null_index};
  index_type m_tail{null_index};
};

// Ensure ordered_index_set adheres to the input range concept.
static_assert(std::ranges::input_range<ordered_index_set<int>>);
static_assert(std::ranges::input_range<const ordered_index_set<int>>);
static_assert(std::ranges::input_range<ordered_index_set<int, 10>>);
static_assert(std::ranges::input_range<const ordered_index_set<int, 10>>);
