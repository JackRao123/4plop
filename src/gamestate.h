#pragma once
#include <algorithm>
#include <exception>
#include <iostream>
#include <utility>
#include <vector>

#include "deck.h"
#include "equity_calc.h"
#include "player.h"

using namespace std;

// CHECK = CHECK
// FOLD = FOLD
// CALL = CALL
// POT = POT
// NOTHING = Cant do anything because all in, or folded
enum HandAction { CHECK, FOLD, CALL, POT, NOTHING, MAX_HAND_ACTIONS };
static constexpr const char* HandActionNames[] = {"CHECK", "FOLD", "CALL",
                                                  "POT", "NOTHING"};

inline string to_string(HandAction a) {
  if (a >= 0 && a < MAX_HAND_ACTIONS) return HandActionNames[a];
  return "UNKNOWN";
}

inline ostream& operator<<(ostream& os, HandAction a) {
  return os << to_string(a);
}

class GameState {
 public:
  // Invariant: board1_[0] < board2_[0].
  Deck deck;
  vector<int> board1_;
  vector<int> board2_;
  int num_players_;
  double stack_depth_;
  double ante_;

  // players_
  vector<Player> players_;
  int next_to_act_ = 0;  // index of player whose turn it is, at this node. Note
                         // that Small Blind = 0.

  // Betting
  double pot_ = 0.0;
  vector<double> bets_placed_;  // money put into the pot in the current round
                                // by each player.
  vector<bool> actioned_;  // whether or not the player has made their required
  // action this round. folded means they have actioned_.
  // when action is reopened, players_ still in the hand
  // have actioned_ = false. (action is re-requested).

  // idx of the previous agressor. (-1 if no aggression this round yet)
  int previous_aggressor_ = -1;

  GameState() {}
  GameState(const vector<int>& board1, const vector<int>& board2,
            int num_players, double stack_depth, double ante)
      : board1_(board1),
        board2_(board2),
        num_players_(num_players),
        stack_depth_(stack_depth),
        ante_(ante) {
    deck.erase(board1_);
    deck.erase(board2_);
    swap_boards_if_necessary();

    for (int i = 0; i < num_players_; i++) {
      add_player(stack_depth_, ante_);
    }
  }

  void swap_boards_if_necessary() {
    if (board1_[0] > board2_[0]) {
      swap(board1_, board2_);
    }
  }

  // resets game state back to the flop
  void reset() {
    deck = Deck();
    deck.erase(board1_);
    deck.erase(board2_);

    players_.clear();
    next_to_act_ = 0;
    pot_ = 0.0;
    bets_placed_.clear();
    actioned_.clear();
    previous_aggressor_ = -1;

    for (int i = 0; i < num_players_; i++) {
      add_player(stack_depth_, ante_);
    }
  }

  void add_player(double stack_depth, double ante) {
    players_.push_back(
        Player(deck.deal_with_modification(4), stack_depth - ante));
    pot_ += ante;

    bets_placed_.push_back(0.0);
    actioned_.push_back(false);
  }

  // Whether or not we are at game end (termination).
  bool end_of_game() {
    if (board1_.size() == 5 && end_of_action()) {
      return true;
    }

    if (everyone_folded_to_aggressor()) {
      return true;
    }

    return false;
  }

  // whether everyone has folded (except 1 person).
  bool everyone_folded_to_aggressor() {
    for (int i = 0; i < players_.size(); i++) {
      if (!players_[i].is_folded() && previous_aggressor_ != i) {
        return false;
      }
    }

    return true;
  }

  // whether or not we are at the end of action for a betting round
  // (flop/turn/river)
  bool end_of_action() {
    for (int i = 0; i < players_.size(); i++) {
      if (!actioned_[i]) {
        return false;
      }
    }

    return true;
  }

  // reopen_action should be called when aggression is made.
  // on all unfolded players_
  void reopen_action(int aggressor) {
    for (int i = 0; i < players_.size(); i++) {
      if (i == aggressor) {
        continue;
      }

      if (!players_[i].is_folded() && !players_[i].is_all_in()) {
        actioned_[i] = false;
      }
    }
  }

  // Calculates the amount required for player to call.
  double calculate_call(int player_idx) {
    if (previous_aggressor_ == -1) {
      return 0.0;
    }

    return bets_placed_[previous_aggressor_] - bets_placed_[player_idx];
  }

  // Calculates the amount for player_idx required to pot/repot (same thing)
  double calculate_pot_bet(int player_idx) {
    // Repot Size = pot Size + Call Amount + Raise Amount
    // Call the largest bet, and then bet pot value.
    double call_amount = calculate_call(player_idx);
    double new_pot = call_amount + pot_;
    double pot_bet = new_pot + call_amount;
    return pot_bet;
  }

  // Deals out one card to each board.
  // Sets first to act to SB.
  // returns {card dealt to board1_, card dealt to board2_}
  pair<int, int> next_street() {
    int c1 = deck.deal();
    int c2 = deck.deal();

    board1_.push_back(c1);
    board2_.push_back(c2);

    for (int i = 0; i < players_.size(); i++) {
      pot_ += bets_placed_[i];
      bets_placed_[i] = 0;

      if (!players_[i].is_folded() && !players_[i].is_all_in()) {
        actioned_[i] = false;
      }
    }

    next_to_act_ = 0;
    while (next_to_act_ < players_.size() &&
           players_[next_to_act_].is_folded()) {
      next_to_act_++;
    }
    if (next_to_act_ == players_.size()) {
      throw exception(
          "All players are folded but next street was still dealt.");
    }

    previous_aggressor_ = -1;
    return {c1, c2};
  }

  // calculates who is next to act, and sets next_to_act_ to that person.
  void set_next_to_act() {
    if (end_of_action()) {
      for (int i = 0; i < num_players_; i++) {
        if (!players_[i].is_folded()) {
          next_to_act_ = i;
          return;
        }
      }
    } else {
      int idx = next_to_act_;
      for (int i = 1; i <= num_players_; i++) {
        idx = (next_to_act_ + i) % num_players_;

        if (!players_[idx].is_folded() && !actioned_[idx]) {
          next_to_act_ = idx;
          return;
        }
      }
    }
  }

  int get_next_to_act() const { return next_to_act_; }

  // Performs a given action, on the current node.
  void do_next_action(HandAction action) {
    Player& player = players_[next_to_act_];  // MUST BE A REFERENCE

    switch (action) {
      case NOTHING:
        // do nothing.
        break;

      case CHECK:
        // Do nothing.
        break;
      case FOLD:
        player.fold();
        break;
      case CALL: {
        double cost = min(player.get_money(), calculate_call(next_to_act_));

        bets_placed_[next_to_act_] += cost;
        player.subtract_money(cost);
        pot_ += cost;
        break;
      }
      case POT: {
        double cost = min(player.get_money(), calculate_pot_bet(next_to_act_));

        bets_placed_[next_to_act_] += cost;
        player.subtract_money(cost);

        previous_aggressor_ = next_to_act_;
        reopen_action(next_to_act_);

        pot_ += cost;
        break;
      }

      default:
        break;
    }

    actioned_[next_to_act_] = true;
    set_next_to_act();
  }

  // Only works if this is a terminal node. Calculates the ev of each player.
  // ev is the amount of chips that they should have at showdown.
  // terminal nodes aren't just river - if everyone folds on the flop to an
  // agressor that is also a terminal node.
  vector<double> calculate_ev(bool debug = false) {
    // deal out the remainder of the board if necessary
    while (board1_.size() != 5) {
      next_street();
    }

    vector<vector<int>> hands;

    for (auto& player : players_) {
      if (!player.is_folded()) {
        hands.push_back(player.get_hand());
      }
    }

    vector<double> equities = equity_calc(hands, board1_, board2_);
    vector<double> evs(players_.size());

    int i = 0;
    for (int j = 0; j < players_.size(); j++) {
      if (!players_[j].is_folded()) {
        evs[j] = equities[i] * pot_;
        i++;
      } else {
        evs[j] = 0.0;
      }
    }

    if (debug) {
      for (int j = 0; j < evs.size(); j++) {
        cout << "Player " << j << " ev: " << evs[j] << endl;
      }
    }
    return evs;
  }

  // Obtains the uniform (default) strategy for the current next-to-act player.
  // Each allowed action has equal probability
  vector<pair<HandAction, double>> GetUniformStrategy() {
    Player player = players_[next_to_act_];

    if (player.is_folded() || player.is_all_in()) {
      return {{NOTHING, 1.0}};
    }

    vector<HandAction> available;

    // pot/repot (same thing).
    // player can reraise as long as they can afford more than just calling.
    if (player.get_money() >= calculate_call(next_to_act_)) {
      available.push_back(POT);
    }

    if (previous_aggressor_ == -1) {
      available.push_back(CHECK);
    } else {
      available.push_back(FOLD);
      available.push_back(CALL);
    }

    vector<pair<HandAction, double>> strat;
    for (const auto& action : available) {
      strat.push_back({action, 1.0 / (double)available.size()});
    }

    return strat;
  }
};
