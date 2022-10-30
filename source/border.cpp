#include "border.hpp"

void update(const subtree_type &sub, border_type &border, const vertex_id id,
            history_type &history) {
  /*
  for each neighborhood node y of x do // increasing ordering of y's ID
    if cnt(S, y) > 1 then
      Remove y from B(S);
      H.push(del, y);
    else if y > root(S) and y is not in S then
      Add y to B(S) on the head;
      H.push(add, y);
    end if
  end for
  */

  history.emplace(action_type::stop, 0);
  for (const auto neighbor : sub.base().vertices[id].neighbors) {
    if (sub.cnt(neighbor) > 1) {
      if (border.remove(neighbor)) {
        history.emplace(action_type::rem, neighbor);
      }
    } else if (neighbor > sub.root() && !sub.has(neighbor)) {
      border.push_front(neighbor);
      history.emplace(action_type::add, neighbor);
    }
  }
}

void restore(border_type &border, history_type &history) {
  /*
  while true do
    (op, x) <- H.top();
    H.pop();
    if op == pivot then
      break;
    else
      B(S).op(x);
    end if
  end while
  */

  while (true) {
    const auto [op, id] = history.top();
    history.pop();

    switch (op) {
    case action_type::add: {
      border.push_front(id);
      break;
    }
    case action_type::rem: {
      border.remove(id);
      break;
    }
    case action_type::stop: {
      return;
    }
    }
  }
}
