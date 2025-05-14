// chancenode.h
#pragma once
#include <unordered_map>
#include <utility>

#include "gamestate.h"
#include "node.h"

using namespace std;
struct PairHash {
  size_t operator()(const std::pair<int, int>& p) const noexcept {
    // very simple hash function
    return (static_cast<size_t>(p.first) << 32) ^ static_cast<size_t>(p.second);
  }
};
struct PairEq {
  bool operator()(const std::pair<int, int>& a,
                  const std::pair<int, int>& b) const noexcept {
    return a.first == b.first && a.second == b.second;
  }
};

// ChanceNode allows us to handle turns and rivers
class ChanceNode : public Node {
 private:
  unordered_map<pair<int, int>, Node*, PairHash, PairEq> next_;

 public:
  ChanceNode(int table_position) : Node(table_position) {}
  // GetNextNodeAndState advances both the game state and the current node, by
  // dealing out the next street
  Node* GetNextNodeAndState(GameState* game_state);

  Node* GetNextNode(int next_top_card, int next_bottom_card);
};
