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

// subtree_generator
// modified_rec_parallel(std::vector<border_type> &border_cache,
//                       thread_pool_type &pool, subtree_type &sub,
//                       border_type &border, history_type &history,
//                       std::invocable<subtree_type> auto &action) {
//   //
// }

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

} // namespace detail

/**
 * @brief Enumerates all induced subtrees of a graph and applies an action on
 * the range it produces, in parallel.
 * @param graph The graph to enumerate over.
 * @param action The action to perfom on the range of induced subtrees. Should
 * either be stateless or thread-safe.
 * @return The result of applying action to all the induced subtrees of graph.
 */
/*
subtree_type
enumerate_recursive(const graph_type &graph,
                    std::invocable<subtree_generator> auto &&action) requires
    std::same_as<std::invoke_result_t<decltype(action), subtree_generator>,
                 subtree_type> {
  // lmrtfy::thread_pool pool;
  //
  // co_yield subtree_type{graph};
  //
  //// Rather than passing by value, as items are removed from the border they
  /// are / placed into one of these corresponding with the size of the subtree.
  /// It is / then swapped back before returning.
  // std::vector<border_type> border_cache(
  //     graph.vertices.size() + 1,
  //     border_type{static_cast<vertex_id>(graph.vertices.size())});
  //
  // for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
  //  subtree_type sub{graph, i};
  //
  //  border_type border(static_cast<vertex_id>(graph.vertices.size()));
  //  history_type history;
  //
  //  update(sub, border, i, history);
  //  co_yield modified_rec(sub, border, history, border_cache);
  //}
}
*/

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
