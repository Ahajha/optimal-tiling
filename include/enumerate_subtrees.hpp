#pragma once

#include "border.hpp"
#include "config.hpp"
#include "enumerate_subtrees.hpp"

#include <cppcoro/recursive_generator.hpp>
#include <lmrtfy/thread_pool.hpp>

using subtree_generator = cppcoro::recursive_generator<subtree_type>;

subtree_generator enumerate(graph_type graph);

namespace detail {
using thread_pool_type =
    lmrtfy::thread_pool<lmrtfy::per_thread<std::vector<border_type>>,
                        lmrtfy::pool_ref>;

/*
Modified rec needs:
border cache& (for normal operation) (from thread pool)
thread pool& (for dispatching) (from thread pool)
subtree& (for normal operation)
border& (for normal operation)
history& (for normal operation)
action& (for dispatching (if applied to a range) or for use (if applied to each
item))

First implementation will be the 'void' version

(We can try to optimize by lumping things into a struct later, for now just pass
everything)
*/

template <std::invocable<subtree_type> TAction>
void modified_rec_trampoline_void(std::vector<border_type> &border_cache,
                                  thread_pool_type &pool, subtree_type sub,
                                  border_type border, history_type history,
                                  TAction action);

template <std::invocable<subtree_type> TAction>
void modified_rec_parallel_void(std::vector<border_type> &border_cache,
                                thread_pool_type &pool, subtree_type &sub,
                                border_type &border, history_type &history,
                                TAction &action) {
  action(sub);

  auto &cache = border_cache[sub.n_induced()];

  while (!border.empty()) {
    auto id = border.pop_front();
    cache.push_back(id);

    sub.add(id);

    update(sub, border, id, history);
    if (pool.n_idle() > 0) {
      // Thread is available
      // Border cache and pool will be passed by the thread dispatcher.
      // Not sure about ownership here... Might need to work on LMRTFY. For now,
      // make the copies verbose. // TODO
      pool.push(modified_rec_trampoline_void<TAction>, subtree_type(sub),
                border_type(border), history_type(history), TAction(action));
    } else {
      // Continue on this thread
      modified_rec_parallel_void(border_cache, pool, sub, border, history,
                                 action);
    }
    restore(border, history);

    sub.rem(id);
  }

  std::swap(cache, border);
}

/*
Trampoline needs:
border cache& (for passing to modified rec)
thread pool& (for passing to modified rec for dispatching)
subtree (for passing to modified rec for normal operation)
border (for passing to modified rec for normal operation)
history (for passing to modified rec for normal operation)
action (for passing to modified rec (if applied to each item, will need to pass
regardless) or for use (if applied to a range))
*/
template <std::invocable<subtree_type> TAction>
void modified_rec_trampoline_void(std::vector<border_type> &border_cache,
                                  thread_pool_type &pool, subtree_type sub,
                                  border_type border, history_type history,
                                  TAction action) {
  // The first time this cache is used, it will be empty. In that case, set it
  // up, otherwise leave it as is.
  if (border_cache.empty()) {
    const auto n_vertices = sub.base_verts().size();
    border_cache.resize(n_vertices + 1,
                        border_type{static_cast<vertex_id>(n_vertices)});
  }

  // Iterate over each subtree and apply the action
  // for (const auto &sub :
  //     modified_rec_parallel(border_cache, pool, sub, border, history)) {
  //  action(sub);
  //}

  // modified_rec_parallel_void applies the action for us
  modified_rec_parallel_void(border_cache, pool, sub, border, history, action);
}

////////////////////// NONVOID /////////////////////////

template <class TAction>
using futures_container_type = std::vector<
    std::future<std::invoke_result_t<TAction, subtree_generator &>>>;

// Forward declaration - the trampoline and the implementation are mutually
// recursive
template <std::invocable<subtree_generator &> TAction>
auto modified_rec_trampoline_nonvoid(
    std::vector<border_type> &border_cache, thread_pool_type &pool,
    subtree_type sub, border_type border, history_type history, TAction action,
    futures_container_type<TAction> &intermediate_futures,
    std::mutex &futures_mut)
    -> std::invoke_result_t<TAction, subtree_generator &>;

template <std::invocable<subtree_generator &> TAction>
subtree_generator modified_rec_parallel_nonvoid(
    std::vector<border_type> &border_cache, thread_pool_type &pool,
    subtree_type &sub, border_type &border, history_type &history,
    TAction &action, futures_container_type<TAction> &intermediate_futures,
    std::mutex &futures_mut) {
  co_yield sub;

  auto &cache = border_cache[sub.n_induced()];

  while (!border.empty()) {
    auto id = border.pop_front();
    cache.push_back(id);

    sub.add(id);

    update(sub, border, id, history);
    if (pool.n_idle() > 0) {
      // Thread is available
      // Border cache and pool will be passed by the thread dispatcher.
      // Not sure about ownership here... Might need to work on LMRTFY. For now,
      // make the copies verbose. // TODO
      auto fut =
          pool.push(modified_rec_trampoline_nonvoid<TAction>, subtree_type(sub),
                    border_type(border), history_type(history), TAction(action),
                    std::ref(intermediate_futures), std::ref(futures_mut));

      std::scoped_lock lock{futures_mut};
      intermediate_futures.push_back(std::move(fut));
    } else {
      // Continue on this thread
      co_yield modified_rec_parallel_nonvoid(border_cache, pool, sub, border,
                                             history, action,
                                             intermediate_futures, futures_mut);
    }
    restore(border, history);

    sub.rem(id);
  }

  std::swap(cache, border);
}

template <std::invocable<subtree_generator &> TAction>
auto modified_rec_trampoline_nonvoid(
    std::vector<border_type> &border_cache, thread_pool_type &pool,
    subtree_type sub, border_type border, history_type history, TAction action,
    futures_container_type<TAction> &intermediate_futures,
    std::mutex &futures_mut)
    -> std::invoke_result_t<TAction, subtree_generator &> {
  // The first time this cache is used, it will be empty. In that case, set it
  // up, otherwise leave it as is.
  if (border_cache.empty()) {
    const auto n_vertices = sub.base_verts().size();
    border_cache.resize(n_vertices + 1,
                        border_type{static_cast<vertex_id>(n_vertices)});
  }

  // Maintain a clean copy for future branches
  const auto actioncopy = action;

  auto gen = modified_rec_parallel_nonvoid(border_cache, pool, sub, border,
                                           history, actioncopy,
                                           intermediate_futures, futures_mut);
  return action(gen);
}

// A first pass action needs to be invoked on a subtree_generator, as well as
// copied to other threads.
template <class T>
concept valid_first_pass_action =
    std::invocable<T, subtree_generator &> && std::copy_constructible<T>;

} // namespace detail

/**
 * @brief Enumerates all induced subtrees of a graph and applies two passes of
 * actions on the range it produces, in parallel.
 * @param graph The graph to enumerate over.
 * @param first_pass_action The action to perfom on the range of induced
 * subtrees produced by each thread. A copy of the original will be passed to
 * each thread and will be invoked in a thread-safe manner.
 * @param second_pass_action The action to perform on the range of objects
 * produced by running the first pass action on each thread. Will be invoked in
 * a thread-safe manner.
 * @return The result of the second pass action
 */
template <detail::valid_first_pass_action TFirstPassAction,
          std::invocable<std::vector<
              std::invoke_result_t<TFirstPassAction, subtree_generator &>> &>
              TSecondPassAction>
auto enumerate_recursive(const graph_type &graph, TFirstPassAction action1,
                         TSecondPassAction action2)
    -> std::invoke_result_t<TSecondPassAction,
                            std::vector<std::invoke_result_t<
                                TFirstPassAction, subtree_generator &>>> {
  const auto action1copy = action1;

  using intermediate_result_type =
      std::invoke_result_t<TFirstPassAction, subtree_generator &>;

  detail::futures_container_type<TFirstPassAction> intermediate_futures;

  {
    std::mutex futures_mut;

    detail::thread_pool_type pool;

    // Rather than passing by value, as items are removed from the border they
    // are placed into one of these corresponding with the size of the subtree.
    // It is then swapped back before returning.
    std::vector<border_type> border_cache(
        graph.vertices.size() + 1,
        border_type{static_cast<vertex_id>(graph.vertices.size())});

    for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
      subtree_type sub{graph, i};

      border_type border(static_cast<vertex_id>(graph.vertices.size()));
      history_type history;

      update(sub, border, i, history);
      auto fut =
          pool.push(detail::modified_rec_trampoline_nonvoid<TFirstPassAction>,
                    subtree_type(sub), border_type(border),
                    history_type(history), TFirstPassAction(action1copy),
                    std::ref(intermediate_futures), std::ref(futures_mut));

      std::scoped_lock lock{futures_mut};
      intermediate_futures.push_back(std::move(fut));
    }

    // Wait for all threads to finish.
  }
  // At this point, it is safe to access the container of futures.

  // Make a simple generator to convert the empty subtree to a coroutine with a
  // single element.
  auto empty_subtree_generator = [&graph]() -> subtree_generator {
    co_yield subtree_type{graph};
  };

  auto gen = empty_subtree_generator();

  // Await all futures and return the result of the second pass
  std::vector<intermediate_result_type> intermediate_results{
      action1(gen),
  };

  for (auto &fut : intermediate_futures) {
    intermediate_results.push_back(fut.get());
  }

  return action2(intermediate_results);
}

/**
 * @brief Enumerates all induced subtrees of a graph and applies an action on
 * the each subtree it produces, in parallel.
 * @param graph The graph to enumerate over.
 * @param action The action to perfom on each induced subtree. Should either be
 * stateless or thread-safe.
 */
template <std::invocable<subtree_type> TAction>
void enumerate_recursive(const graph_type &graph, TAction &&action) requires
    std::is_void_v<std::invoke_result_t<decltype(action), subtree_type>> {
  action(subtree_type{graph});

  detail::thread_pool_type pool;

  for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
    subtree_type sub{graph, i};

    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    update(sub, border, i, history);
    pool.push(detail::modified_rec_trampoline_void<TAction>, subtree_type(sub),
              border_type(border), history_type(history), TAction(action));
  }
}
