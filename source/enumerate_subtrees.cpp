#include "enumerate_subtrees.hpp"
#include "border.hpp"
#include "config.hpp"

#include <cppcoro/recursive_generator.hpp>

static cppcoro::recursive_generator<subtree_type>
modified_rec(subtree_type &sub, border_type border, history_type &history) {
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

  while (!border.empty()) {
    auto id = border.pop_front();

    sub.add(id);

    update(sub, border, id, history);
    co_yield modified_rec(sub, border, history);
    restore(border, history);

    sub.rem(id);
  }
}

cppcoro::recursive_generator<subtree_type> enumerate(graph_type graph) {
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

  for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
    subtree_type sub{graph, i};

    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    update(sub, border, i, history);
    co_yield modified_rec(sub, border, history);
  }
}
