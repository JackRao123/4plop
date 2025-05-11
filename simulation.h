#ifndef SIMULATION_H
#define SIMULATION_H
#include "equity_calc.h"
#include "include/phevaluator.h"
#include "player.h"
#include "node.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <unordered_set>
#include <vector>

class Simulation {
private:
  int num_players;

public:
  // this calculates the optimal strategy
  // parameters
  // flop1 - top board flop
  // flop2 - bottom board flop
  // num_players - number of players.
  // stack_depth - stack depth, in $.
  // ante - bomb pot ante, in $.
  Simulation() {}

  // CFR is a recursive DFS

  // DFS for CFR:
  // Params:
  // Node: current node
  // Deck: deck of cards
  // Returns:
  // Vector of EVs for each player.
  vector<double> recurse(Node *node) {
    if (node->end_of_game()) {
      // If it is terminal, it is not a decision node.
      // So therefore just return EVs here.

      return node->calculate_ev();
    }

    if (node->end_of_action()) {
      node->next_street();
      return recurse(node);
    }

    // This returns the EVs for all players but we only calculate the regret for
    // hero here. The rest we just pass back.
    vector<double> overall_ev(num_players);
    int hero = node->get_next_to_act();

    // Calculate regret for hero.
    unordered_map<HandAction, double> action_ev;
    unordered_map<HandAction, int> action_count;

    int num_simulations = 1;
    for (int i = 0; i < num_simulations; i++) {
      Node *next = new Node(node);
      HandAction action = next->do_next_action();

      if (node->children.find(next->get_filename()) != node->children.end()) {
        next = node->children[next->get_filename()];
      } else {
        node->children[next->get_filename()] = next;
      }

      vector<double> ev = recurse(next);
      for (int j = 0; j < num_players; j++) {
        overall_ev[j] += ev[j];
      }

      action_ev[action] += ev[hero];
      action_count[action]++;
    }

    // Convert to average
    for (int i = 0; i < num_players; i++) {
      overall_ev[i] /= (double)num_simulations;
    }

    // Convert to average
    for (const auto &[action, count] : action_count) {
      action_ev[action] /= (double)count;
    }

    // This is the strategy for 'next_to_act', at the current NODE.
    node->adjust_strategy(action_ev, hero);

    return overall_ev;
  }

  // Entry point
  void simulate(const vector<int> &flop1, const vector<int> &flop2, int num_players, double stack_depth, double ante) {
    this->num_players = num_players;

    Node *head = new Node(flop1, flop2);
    for (int i = 0; i < num_players; i++) {
      head->add_player(stack_depth, ante);
    }

    for (int i = 0; i < 10000; i++) {
      head->redeal(); // re-deals everyone cards.
      recurse(head);
      cout << i << endl;
    }

    // Deal out cards randomly.
    // Recurse.
    // If a particular node is terminal, then calculate the amount of money the
    // hand gets. Adjust regrets accordingly.

    // For all possible actions from head:
    // Recurse along those
    // If a particular node is terminal, then return the amount of money that
    // each hand gets.

    cout << "OK" << endl;
    cout << head->strategy.size() << endl;

    // vector<pair<HandAction, double>> strat = head->strategy[hand_hash(string_to_hand("8d8cKhKs"))];
    // vector<pair<HandAction, double>> strat = head->strategy[hand_hash(string_to_hand("ThAh3s4s"))];

    for (const auto &[hand_hash, strat] : head->strategy) {
      cout << "Hand: " << hand_hash_to_string(hand_hash) << endl;
      for (const auto &[action, probability] : strat) {
        cout << "Action " << action << " taken with probability " << probability << endl;
      }
    }
  }
};

#endif