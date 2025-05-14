// chancenode.cpp
#include "chancenode.h"

#include <algorithm>

#include "gamestate.h"
#include "node.h"

using namespace std;


Node* ChanceNode::GetNextNodeAndState(GameState* game_state) {
  pair<int, int> dealt_cards = game_state->next_street();

  Node* child;
  if (next_.find(dealt_cards) != next_.end()) {
    child = next_[dealt_cards];
  } else {
    child = new Node(game_state->get_next_to_act());
    child->parent = this;
    next_[dealt_cards] = child;
  }

  // have to update the state no matter what -
  // even though action sequences are the same, we must change what cards the
  // players have between runs.
  // child->state_ = next_state;
  return child;
}

Node* ChanceNode::GetNextNode(int next_top_card, int next_bottom_card) {
  return next_[{next_top_card, next_bottom_card}];
}