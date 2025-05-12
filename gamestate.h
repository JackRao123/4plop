#ifndef GAMESTATE_H
#define GAMESTATE_H
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


using namespace std;


// CHECK = CHECK
// FOLD = FOLD
// CALL = CALL
// POT = POT
// NOTHING = Cant do anything because all in, or folded
enum HandAction { CHECK, FOLD, CALL, POT, NOTHING, MAX_HAND_ACTIONS };
static constexpr const char* HandActionNames[] = { "CHECK", "FOLD", "CALL", "POT", "NOTHING" };

inline string to_string(HandAction a) {
	if (a >= 0 && a < MAX_HAND_ACTIONS)
		return HandActionNames[a];
	return "UNKNOWN";
}

inline ostream& operator<<(ostream& os, HandAction a) { return os << to_string(a); }



class GameState {
public:
	// Invariant: board1[0] < board2[0].
	vector<int> board1;
	vector<int> board2;
	Deck deck;

	// Players
	vector<Player> players;
	int next_to_act = 0; // index of player whose turn it is, at this node. Note that Small Blind = 0.


	// Betting
	double pot = 0.0;
	vector<double> bets_placed; // money put into the pot in the current round by each player.
	vector<bool> actioned; // whether or not the player has made their required
	// action this round. folded means they have actioned.
	// when action is reopened, players still in the hand
	// have actioned = false. (action is re-requested).

	// idx of the previous agressor. (-1 if no aggression this round yet)
	int previous_aggressor = -1;

	GameState() {}
	GameState(const vector<int>& board1, const vector<int>& board2, int num_players, double stack_depth, double ante) {
		this->board1 = board1;
		this->board2 = board2;
		deck.erase(board1);
		deck.erase(board2);
		swap_boards_if_necessary();

		for (int i = 0; i < num_players; i++) {
			add_player(stack_depth, ante);
		}
	}

	void swap_boards_if_necessary() {
		if (board1[0] > board2[0]) {
			swap(board1, board2);
		}
	}


	// re-deals everyone
	void redeal() {
		deck = Deck();
		deck.erase(board1);
		deck.erase(board2);

		for (int i = 0; i < players.size(); i++) {
			players[i].hand = deck.deal_with_modification(4);
		}
	}

	void add_player(double stack_depth, double ante) {
		players.push_back(Player(deck.deal_with_modification(4), stack_depth - ante));
		pot += ante;

		bets_placed.push_back(0.0);
		actioned.push_back(false);
	}


	// Whether or not we are at game end (termination).
	bool end_of_game() { return board1.size() == 5 && end_of_action(); }

	// whether or not we are at the end of action for a betting round
	// (flop/turn/river)
	bool end_of_action() {
		for (int i = 0; i < players.size(); i++) {
			if (!actioned[i]) {
				return false;
			}
		}

		return true;
	}


	// reopen_action should be called when aggression is made.
// on all unfolded players
	void reopen_action(int aggressor) {
		for (int i = 0; i < players.size(); i++) {
			if (i == aggressor) {
				continue;
			}

			if (!players[i].is_folded() && !players[i].is_all_in()) {
				actioned[i] = false;
			}
		}
	}


	// Calculates the amount required for player to call.
	double calculate_call(int player_idx) {
		if (previous_aggressor == -1) {
			return 0.0;
		}

		return bets_placed[previous_aggressor] - bets_placed[player_idx];
	}

	// Calculates the amount for player_idx required to pot/repot (same thing)
	double calculate_pot_bet(int player_idx) {
		// Repot Size = Pot Size + Call Amount + Raise Amount
		// Call the largest bet, and then bet pot value.
		double call_amount = calculate_call(player_idx);
		double new_pot = call_amount + pot;
		double pot_bet = new_pot + call_amount;
		return pot_bet;
	}


	// Deals out one card to each board.
	// Sets first to act to SB.
	void next_street() {
		board1.push_back(deck.deal());
		board2.push_back(deck.deal());

		for (int i = 0; i < players.size(); i++) {
			pot += bets_placed[i];
			bets_placed[i] = 0;

			if (!players[i].is_folded() && !players[i].is_all_in()) {
				actioned[i] = false;
			}
		}

		next_to_act = 0;
		previous_aggressor = -1;
	}

	int get_next_to_act() const { return next_to_act; }





	// Performs a given action, on the current node.
	void do_next_action(HandAction  action) {
		Player& player = players[next_to_act]; // MUST BE A REFERENCE


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
			double cost = min(player.get_money(), calculate_call(next_to_act));

			bets_placed[next_to_act] += cost;
			player.subtract_money(cost);
			pot += cost;
			break;
		}
		case POT: {
			double cost = min(player.get_money(), calculate_pot_bet(next_to_act));

			bets_placed[next_to_act] += cost;
			player.subtract_money(cost);

			previous_aggressor = next_to_act;
			reopen_action(next_to_act);

			pot += cost;
			break;
		}

		default:
			break;
		}

		actioned[next_to_act] = true;
		next_to_act = (next_to_act + 1) % players.size();
	}



	// Only works if this is a terminal node. Calculates the ev of each player.
	// ev is the amount of chips that they should have at showdown.
	vector<double> calculate_ev(bool debug = false) {
		vector<vector<int>> hands;

		for (auto& player : players) {

			if (!player.is_folded()) {
				hands.push_back(player.get_hand());
			}
		}

		vector<double> equities = equity_calc(hands, board1, board2);

		vector<double> evs(players.size());

		int i = 0;
		for (int j = 0; j < players.size(); j++) {
			if (!players[j].is_folded()) {
				evs[j] = equities[i] * pot;

				i++;
			}
		}



		return evs;
	}






};

#endif