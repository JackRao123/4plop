// node.cpp
#include "node.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <thread>

#include "chancenode.h"
#include "equity_calc.h"
#include "include/phevaluator.h"

using namespace std;

// Adjust the strategy.
// action_ev is the ev of various actions, performed by player_idx at this
// node.
void Node::AdjustStrategy(GameState* game_state,
                          unordered_map<HandAction, double>& action_ev,
                          int handhash, double reach_probability) {
  vector<pair<HandAction, double>> strat = GetStrategy(game_state, handhash);

  // weighted ev of this strategy.
  double strategy_ev = 0.0;
  for (const auto& [action, probability] : strat) {
    strategy_ev += probability * action_ev[action];
  }

  // Update regrets for each action
  for (auto& [action, probability] : strat) {
    double regret = action_ev[action] - strategy_ev;  // CFR Regret formula
    cumulative_regret_[handhash][action] += regret;
  }

  for (auto& [action, probability] : strat) {
    cumulative_strategy_[handhash][action] += probability * reach_probability;
  }

  double sum_pos_regret = 0.0;
  for (auto& pr : cumulative_regret_[handhash]) {
    if (pr.second > 0) {
      sum_pos_regret += pr.second;
    }
  }

  if (sum_pos_regret > 0) {
    strat.clear();
    for (const auto& [action, r] : cumulative_regret_[handhash]) {
      double p = r > 0 ? r / sum_pos_regret : 0;
      strat.push_back({action, p});
    }
    strategy[handhash] = strat;
  } else {  // fall back to default
    strategy[handhash] = game_state->GetUniformStrategy();
  }

  visit_count_[handhash] += reach_probability;
}

// GetStrategy finds the strategy for a particular player at this node, or sets
// it to the uniform strategy if it doesn't exist.
vector<pair<HandAction, double>> Node::GetStrategy(GameState* game_state,
                                                   int handhash) {
  if (strategy.find(handhash) == strategy.end()) {
    strategy[handhash] = game_state->GetUniformStrategy();
  }

  return strategy[handhash];
}

// Randomises next action based on strategy probabilities.
// Doesn't perform the action.
// Returns {action to be performed, probability of choosing this action}.
pair<HandAction, double> Node::GetNextAction(GameState* game_state,
                                             int handhash) {
  vector<pair<HandAction, double>> strat = GetStrategy(game_state, handhash);

  double chosen = rand_double(0.0, 1.0);
  double cumulative = 0.0;
  for (const auto& [action, probability] : strat) {
    cumulative += probability;

    if (chosen <= cumulative) {
      return {action, probability};
    }
  }

  return strat.back();
}

Node* Node::GetNextNodeAndState(GameState* game_state, HandAction action) {
  // GameState next_state = state_;
  game_state->do_next_action(action);

  Node* child;
  if (children.find(action) != children.end()) {
    child = children[action];
  } else {
    if (game_state->end_of_action()) {
      child = new ChanceNode(game_state->get_next_to_act());
    } else {
      child = new Node(game_state->get_next_to_act());
    }
    child->parent = this;
    children[action] = child;
  }

  // have to update the state no matter what -
  // even though action sequences are the same, we must change what cards the
  // players have between runs.
  // child->state_ = next_state;
  return child;
}

// Returns position on table like UTG, BTN
// only works for 6-handed right now
string Node::GetTablePosition() const {
  static constexpr std::array<const char*, 6> positions = {"SB", "BB", "UTG",
                                                           "HJ", "CO", "BTN"};
  return positions[table_position_];
}
