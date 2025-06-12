// node.h
#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "gamestate.h"

#define MAX_UNIQUE_HANDS 52 * 52 * 52 * 52

using namespace std;
class Node {
  // Define a node based on
  // 1. Board
  // 2. Action sequence

 public:
  // Strategy for each hand.
  // Define the strategy as the strategy for the person that action is on
  // currently. Maps handhash to strategy. each pair<HandAction, double>
  // represents the probability at which we do that action.
  unordered_map<int, vector<pair<HandAction, double>>> strategy;

  // concurrency
  mutex mtx;

  // Cumulative positive regret for each info set (handhash -> action -> regret)
  unordered_map<int, unordered_map<HandAction, double>> cumulative_regret_;

  // Cumulative strategy sums for averaging (handhash -> action -> sum of
  // prob_choice*reach)
  unordered_map<int, unordered_map<HandAction, double>> cumulative_strategy_;

  // How often you visited this info set (handhash -> total reach probability)
  unordered_map<int, double> visit_count_;

  // Traversal
  unordered_map<HandAction, Node*> children;
  Node* parent = nullptr;

  // Game state. Should be copied (and then modified) when spawning children.
  // GameState state_;

  int table_position_;

  Node(int table_position) : table_position_(table_position) {}
  // Node(const vector<int>& board1, const vector<int>& board2, int num_players,
  //      double stack_depth, double ante) {
  //   state_ = GameState(board1, board2, num_players, stack_depth, ante);
  // }

  virtual ~Node() = default;

  void AdjustStrategy(GameState* game_state, unordered_map<HandAction, double>& action_ev, int handhash, double reach_probability);

  // Gets the strategy for a particular hand at this node
  vector<pair<HandAction, double>> GetStrategy(GameState* game_state, int handhash);

  // Randomises next action based on strategy probabilities.
  // Doesn't perform the action.
  // Returns {action to be performed, probability of choosing this action}.
  pair<HandAction, double> GetNextAction(GameState* game_state, int handhash);

  // GetNextNodeAndState advances both the game state, and the current node, by
  // performing an action.
  Node* GetNextNodeAndState(GameState* game_state, HandAction action);

  // Returns position on table like UTG, BTN
  // only works for 6-handed right now
  string GetTablePosition() const;
};
