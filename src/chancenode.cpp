// chancenode.cpp
#include "chancenode.h"

#include <algorithm>

#include "gamestate.h"
#include "node.h"

using namespace std;

// GetChild deals out the next street, and then returns a decision node.
Node* ChanceNode::GetChild() {
  GameState next_state = state_;
  pair<int, int> dealt_cards = next_state.next_street();

  Node* child;
  if (next_.find(dealt_cards) != next_.end()) {
    child = next_[dealt_cards];
  } else {
    child = new Node();
    child->parent = this;
    next_[dealt_cards] = child;
  }

  // have to update the state no matter what -
  // even though action sequences are the same, we must change what cards the
  // players have between runs.
  child->state_ = next_state;
  return child;
}

Node* ChanceNode::GetNextNode(int next_top_card, int next_bottom_card) {
  return next_[{next_top_card, next_bottom_card}];
}