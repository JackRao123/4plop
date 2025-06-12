#pragma once

#include <chrono>
#include <exception>
#include <iostream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "chancenode.h"
#include "node.h"

class Simulation {
 private:
  enum State { RUNNING, PAUSED, STOPPED };

  int num_players_;

  Node* root_ = nullptr;   // root of the game tree (shouldn't change)
  Node* focus_ = nullptr;  // node from which we are running computations and
                           // are viewing strategy for
  GameState* game_state_;  // gamestate. reset at each iteration, and passed
                           // through the game tree when recursing

  mutex mtx;

  State state_ = State::STOPPED;
  thread worker_thread_;

 public:
  // this calculates the optimal strategy
  // parameters
  // flop1 - top board flop
  // flop2 - bottom board flop
  // num_players - number of players.
  // stack_depth - stack depth, in $.
  // ante - bomb pot ante, in $.
  Simulation() {}

  // Recursive DFS for CFR:
  // Params:
  // Node: node to recurse from.
  // reach_probability: probability of reaching node.
  // Returns:
  // Vector of EVs for each player.
  vector<double> recurse(Node* node, GameState* game_state, double reach_probability) {
    if (game_state->end_of_game()) {
      // If it is terminal, it is not a decision node.
      // So therefore just return EVs here.
      return game_state->calculate_ev();
    }

    if (auto chance_node = dynamic_cast<ChanceNode*>(node)) {
      // next_node is a decision node - after dealing cards.
      Node* next_node = chance_node->GetNextNodeAndState(game_state);
      return recurse(next_node, game_state, reach_probability);
    }

    // This returns the EVs for all players but we only calculate the regret for
    // hero here. The rest we just pass back.
    vector<double> average_ev(num_players_);
    int hero = game_state->get_next_to_act();
    int handhash = hand_hash(game_state->players_[game_state->get_next_to_act()].get_hand());

    // Calculate regret for hero.
    unordered_map<HandAction, double> action_ev;
    unordered_map<HandAction, int> action_count;

    int num_simulations = 1;
    for (int i = 0; i < num_simulations; i++) {
      GameState state_copy = *game_state;

      auto [next_action, action_probability] = node->GetNextAction(&state_copy, handhash);
      Node* next = node->GetNextNodeAndState(&state_copy, next_action);  // advances game_state

      vector<double> sample_ev = recurse(next, &state_copy, reach_probability * action_probability);
      for (int j = 0; j < num_players_; j++) {
        average_ev[j] += sample_ev[j];
      }

      action_ev[next_action] += sample_ev[hero];
      action_count[next_action]++;
    }

    // Convert to average
    for (int i = 0; i < num_players_; i++) {
      average_ev[i] /= (double)num_simulations;
    }

    // Convert to average
    for (const auto& [action, count] : action_count) {
      action_ev[action] /= (double)count;
    }

    // This is the strategy for 'next_to_act', at the current NODE.
    node->AdjustStrategy(game_state, action_ev, handhash, reach_probability);

    return average_ev;
  }

  // Entry point
  void initialise(const string& flop1, const string& flop2, int num_players, double stack_depth, double ante) {
    if (flop1.size() != 6) {
      throw exception("Flop 1 is not correctly specified.");
    }
    if (flop2.size() != 6) {
      throw exception("Flop 2 is not correctly specified.");
    }

    vector<int> flop1vec = string_to_cards(flop1);
    vector<int> flop2vec = string_to_cards(flop2);

    set<int> unique_cards;
    unique_cards.insert(flop1vec.begin(), flop1vec.end());
    unique_cards.insert(flop2vec.begin(), flop2vec.end());

    if (unique_cards.size() != 6) {
      throw exception("The two flops must have 6 unique cards.");
    }

    num_players_ = num_players;

    // Node(flop1vec, flop2vec, num_players, stack_depth,
    // ante);
    game_state_ = new GameState(flop1vec, flop2vec, num_players, stack_depth, ante);
    root_ = new Node(game_state_->get_next_to_act());
    focus_ = root_;
  }

  // you should run this in a separate thread.
  void SolverLoop() {
    int iterations = 0;
    cout << "Starting solver loop " << endl;
    // loop exits on StopSolver();
    unique_lock<mutex> lock(mtx, defer_lock);
    while (true) {
      lock.lock();
      if (state_ == State::STOPPED) {
        lock.unlock();
        break;
      } else if (state_ == State::PAUSED) {
        lock.unlock();
        this_thread::sleep_for(chrono::milliseconds(100));
        continue;
      }

      // drop the lock while running the computation
      iterations++;
      // cout << "Done " << iterations << " iterations " << endl;

      lock.unlock();
      // recurse only from the root.
      // add functionality later for switching recursion basepoint.

      // GameState* state = new GameState()

      // game_state = root_state_;//copy
      // root_->state_.reset();
      game_state_->reset();
      recurse(root_, game_state_, 1.0);
      // focus_->state_.reset();
      // recurse(focus_, 1.0);
    }
  }

  void StartSolver() {
    cout << "Starting solver " << endl;
    ResumeSolver();
    worker_thread_ = thread(&Simulation::SolverLoop, this);
  }

  void ResumeSolver() {
    lock_guard<mutex> lock(mtx);
    state_ = State::RUNNING;
  }

  void PauseSolver() {
    lock_guard<mutex> lock(mtx);
    state_ = State::PAUSED;
  }

  void StopSolver() {
    cout << "stopping solver " << endl;
    {
      lock_guard<mutex> lock(mtx);
      state_ = State::STOPPED;
    }
    worker_thread_.join();
  }

  void SetFocus(Node* new_focus) {
    PauseSolver();
    cout << "Changed focus from " << focus_ << " to " << new_focus << endl;
    cout << "Position: " << new_focus->GetTablePosition() << endl;

    focus_ = new_focus;
    ResumeSolver();
  }

  // GetFocus returns the current focus of the game tree.
  // it is the node for which we are looking at strategy for.
  Node* GetFocus() { return focus_; }

  // GetRoot returns the root of the game tree. The root never changes.
  Node* GetRoot() { return root_; }
};