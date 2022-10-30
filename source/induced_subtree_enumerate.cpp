#include "border.hpp"
#include "config.hpp"

#include <iostream>
#include <list>
#include <stack>

std::ostream &operator<<(std::ostream &stream, const subtree_type &sub) {
  const auto &dims = sub.base().dims_array;
  if (dims.size() == 1) {
    const vertex_id d1 = dims[0];
    for (vertex_id i = 0; i < d1; ++i) {
      stream << (sub.has(i) ? 'X' : '_');
    }
    stream << '\n';
  } else if (dims.size() == 2) {
    const vertex_id d1 = dims[0];
    const vertex_id d2 = dims[1];

    vertex_id index = 0;
    for (vertex_id i = 0; i < d1; ++i) {
      for (vertex_id j = 0; j < d2; ++j) {
        stream << (sub.has(index) ? 'X' : '_');
        ++index;
      }
      stream << '\n';
    }
  } else {
    // TODO
  }
  return stream;
}

void modified_rec(subtree_type &sub, border_type &border,
                  history_type &history) {
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

  std::cout << sub << '\n';

  while (!border.empty()) {
    auto id = border.pop_front();

    sub.add(id);

    update(sub, border, id, history);
    modified_rec(sub, border, history);
    restore(border, history);

    sub.rem(id);
  }
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
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

  graph_type graph{3};

  // output the empty set - ignoring this one

  for (vertex_id i = 0; i < graph.vertices.size(); ++i) {
    subtree_type sub{graph, i};

    border_type border(static_cast<vertex_id>(graph.vertices.size()));
    history_type history;

    update(sub, border, i, history);
    modified_rec(sub, border, history);
  }
}
