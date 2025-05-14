#pragma once

#include <iostream>
#include <vector>

#include "deck.h"
#include "helper.h"
#include "include/phevaluator.h"

using namespace std;

// Helper function - calculates equity at showdown.
inline vector<double> equity_calc(vector<vector<int>>& hands,
                                  vector<int>& board1, vector<int>& board2) {
  int num_players = hands.size();
  vector<double> equity(num_players);

  phevaluator::Rank board_one_best_rank = phevaluator::Rank(999999);
  phevaluator::Rank board_two_best_rank = phevaluator::Rank(999999);
  vector<int> board_one_winners;
  vector<int> board_two_winners;

  for (int j = 0; j < num_players; j++) {
    phevaluator::Rank board1_j = phevaluator::EvaluatePlo4Cards(
        board1[0], board1[1], board1[2], board1[3], board1[4], hands[j][0],
        hands[j][1], hands[j][2], hands[j][3]);
    phevaluator::Rank board2_j = phevaluator::EvaluatePlo4Cards(
        board2[0], board2[1], board2[2], board2[3], board2[4], hands[j][0],
        hands[j][1], hands[j][2], hands[j][3]);

    if (board1_j > board_one_best_rank) {
      board_one_winners.clear();
      board_one_winners.push_back(j);
      board_one_best_rank = board1_j;
    } else if (board1_j == board_one_best_rank) {
      board_one_winners.push_back(j);
    }

    if (board2_j > board_two_best_rank) {
      board_two_winners.clear();
      board_two_winners.push_back(j);
      board_two_best_rank = board2_j;
    } else if (board2_j == board_two_best_rank) {
      board_two_winners.push_back(j);
    }
  }

  for (const auto& winner : board_one_winners) {
    equity[winner] += 0.5 / (double)(board_one_winners.size());
  }

  for (const auto& winner : board_two_winners) {
    equity[winner] += 0.5 / (double)(board_two_winners.size());
  }

  return equity;
}

// this will probably be the most useful one.
// given that we are at the flop, and I am holding a specific hand, calculate my
// equity. void multiway_equity_calc(my_hand, flop1, flop2)
inline void multiway_equity_calc(vector<int> hand, vector<int> flop1,
                                 vector<int> flop2, Deck d) {
  int num_players = 8;
  int num_iterations = 100000;

  double sum_eq = 0;

  for (int i = 0; i < num_iterations; i++) {
    vector<int> other_cards =
        d.deal_without_modification((num_players - 1) * 4 + 4);

    // If there are num_players players including us, then there are num_players
    // - 1 other players. Cards [0,1,2,3], [4,5,6,7]... belong to those other
    // people.

    // Then the last 4 cards are for the 2 turn and rivers.
    int t1 = other_cards[(num_players - 1) * 4];
    int r1 = other_cards[(num_players - 1) * 4 + 1];
    int t2 = other_cards[(num_players - 1) * 4 + 2];
    int r2 = other_cards[(num_players - 1) * 4 + 3];

    vector<int> board1 = {flop1[0], flop1[1], flop1[2], t1, r1};
    vector<int> board2 = {flop2[0], flop2[1], flop2[2], t2, r2};

    vector<vector<int>> hands;
    hands.push_back(hand);

    for (int j = 0; j < num_players - 1; j++) {
      hands.push_back({other_cards[4 * j], other_cards[4 * j + 1],
                       other_cards[4 * j + 2], other_cards[4 * j + 3]});
    }

    vector<double> showdown_equity = equity_calc(hands, board1, board2);

    sum_eq += showdown_equity[0];
  }

  cout << "Iterations: " << num_iterations << endl;
  cout << "Players: " << num_players << endl;

  cout << "Flop1 : " << hand_to_string(flop1) << endl;
  cout << "Flop2 : " << hand_to_string(flop2) << endl;

  cout << "Hero Hand: " << hand_to_string(hand) << endl;

  cout << "Equity: " << sum_eq / (double)num_iterations << endl;
}

inline void multiway_equity_calc() {
  int num_players = 5;
  int num_iterations = 100000;

  // sum_eq[i] = cumulative equity of player i.
  vector<double> sum_eq(num_players);

  vector<vector<int>> hands(num_players);

  Deck d;
  for (int i = 0; i < num_players; i++) {
    hands[i] = d.deal_with_modification(4);
  }

  for (int i = 0; i < num_iterations; i++) {
    // b = both boards. First 5 is first board, second 5 is second board.
    vector<int> b = d.deal_without_modification(10);

    vector<int> board1 = {b[0], b[1], b[2], b[3], b[4]};
    vector<int> board2 = {b[5], b[6], b[7], b[8], b[9]};
    vector<double> showdown_equity = equity_calc(hands, board1, board2);

    for (int j = 0; j < num_players; j++) {
      sum_eq[j] += showdown_equity[j];
    }
  }

  cout << "Iterations: " << num_iterations << endl;
  cout << "Players: " << num_players << endl;
  for (int i = 0; i < num_players; i++) {
    cout << "Player " << i << ": Hand = " << hand_to_string(hands[i])
         << "| Equity = " << sum_eq[i] / (double)num_iterations << endl;
  }
}

inline int equity_calc() {
  Deck d;

  vector<int> h1 = d.deal_with_modification(4);
  vector<int> h2 = d.deal_with_modification(4);

  double sum_eq1 = 0.0;
  double sum_eq2 = 0.0;

  int num_iterations = 10000;

  int chop1 = 0;
  int chop2 = 0;
  int chopboth = 0;

  for (int i = 0; i < num_iterations; i++) {
    // b = both boards. First 5 is first board, second 5 is second board.
    vector<int> b = d.deal_without_modification(10);

    phevaluator::Rank h1b1 = phevaluator::EvaluatePlo4Cards(
        b[0], b[1], b[2], b[3], b[4], h1[0], h1[1], h1[2], h1[3]);
    phevaluator::Rank h1b2 = phevaluator::EvaluatePlo4Cards(
        b[5], b[6], b[7], b[8], b[9], h1[0], h1[1], h1[2], h1[3]);

    phevaluator::Rank h2b1 = phevaluator::EvaluatePlo4Cards(
        b[0], b[1], b[2], b[3], b[4], h2[0], h2[1], h2[2], h2[3]);
    phevaluator::Rank h2b2 = phevaluator::EvaluatePlo4Cards(
        b[5], b[6], b[7], b[8], b[9], h2[0], h2[1], h2[2], h2[3]);

    // equity of player 1 and player 2. Sum to 1.00
    double eq1 = 0.0;
    double eq2 = 0.0;

    // First board
    if (h1b1 == h2b1) {
      // Chop
      eq1 += 0.25;
      eq2 += 0.25;
      chop1++;
    } else if (h1b1 < h2b1) {
      eq2 += 0.5;
    } else {
      eq1 += 0.5;
    }

    if (h1b2 == h2b2) {
      // Chop
      eq1 += 0.25;
      eq2 += 0.25;
      chop2++;
    } else if (h1b2 < h2b2) {
      eq2 += 0.5;
    } else {
      eq1 += 0.5;
    }

    if (eq1 == eq2) {
      chopboth++;
    }
    sum_eq1 += eq1;
    sum_eq2 += eq2;
  }

  cout << hand_to_string(h1) << " has equity "
       << sum_eq1 / (double)num_iterations << endl;

  cout << hand_to_string(h2) << " has equity "
       << sum_eq2 / (double)num_iterations << endl;

  cout << "Chop board 1 " << chop1 << "/" << num_iterations << " times "
       << endl;

  cout << "Chop board 2 " << chop2 << "/" << num_iterations << " times "
       << endl;

  cout << "Chop both boards " << chopboth << "/" << num_iterations << " times "
       << endl;

  return 0;
}
