#include "config.hpp"

/**
 * @brief Modifies the border to reflect the action of adding a vertex to a
 * subtree, which is assumed to already have happened. Keeps track of the
 * modifications that were performed.
 *
 * @param sub The subtree that was added to
 * @param border The border to update
 * @param id The vertex that was added
 * @param history Used to store the actions that were performed
 */
void update(const subtree_type &sub, border_type &border, const vertex_id id,
            history_type &history);

/**
 * @brief Restores the last state of the border
 *
 * @param border The border to restore
 * @param history The history of changes to the border
 */
void restore(border_type &border, history_type &history);