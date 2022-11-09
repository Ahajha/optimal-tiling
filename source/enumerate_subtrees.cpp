#include "enumerate_subtrees.hpp"
#include "border.hpp"
#include "config.hpp"

#include <cppcoro/recursive_generator.hpp>

#include <utility>

static subtree_generator modified_rec(subtree_type &sub, border_type &border,
                                      history_type &history,
                                      std::vector<border_type> &border_cache) {
  /*
  Output S;
  while B(S) is not empty do
    x <- the head of B(S);
    Remove the head of B(S);
    S <- Ch(S, x);
    H.push(pivot, NIL);
    UPDATE(S, B(S), x, H);
    MODIFIEDREC(S, B(S), H);
    RESTORE(B(S), H);
    S <- P(S)
  end while
  */

  co_yield sub;

  auto &cache = border_cache[sub.n_induced()];

  while (!border.empty()) {
    auto id = border.pop_front();
    cache.push_back(id);

    sub.add(id);

    update(sub, border, id, history);
    co_yield modified_rec(sub, border, history, border_cache);
    restore(border, history);

    sub.rem(id);
  }

  std::swap(cache, border);
}

subtree_generator enumerate(graph_type graph) {
  /*
  Number the nodes of G from 1 to |V| as ID;
  Output {};
  for each node x in G do
    S <- {x};
    B(S) <- an empty doubly linked list;
    H <- an empty stack;
    UPDATE(S, B(S), x, H);
    MODIFIEDREC(S, B(S), H);
  end for
  */
  co_yield subtree_type{graph};

  // Rather than passing by value, as items are removed from the border they are
  // placed into one of these corresponding with the size of the subtree. It is
  // then swapped back before returning.
  std::vector<border_type> border_cache(
      graph.vertices.size() + 1,
      border_type{static_cast<vertex_id>(graph.vertices.size())});

  for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
    subtree_type sub{graph, i};

    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    update(sub, border, i, history);
    co_yield modified_rec(sub, border, history, border_cache);
  }
}
