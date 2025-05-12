#ifndef NODE_H
#define NODE_H
#include "equity_calc.h"
#include "include/phevaluator.h"
#include "player.h"
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

#define MAX_UNIQUE_HANDS 52 * 52 * 52 * 52

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

using namespace std;
class Node {
	// Define a node based on
	// 1. Board
	// 2. Action sequence

public:
	// Invariant: board1[0] < board2[0].
	vector<int> board1;
	vector<int> board2;
	vector<HandAction> actions;
	Deck deck;

	// Strategy for each hand.
	// Define the strategy as the strategy for the next-to-act person.
	// Maps handhash to strategy. each pair<HandAction, double> represents the probability at which we do that action.
	unordered_map<int, vector<pair<HandAction, double>>> strategy;

	// ev of a particular hand at the current node.
	// evaluated only at terminal nodes.
	unordered_map<int, double> ev;

	// Players
	vector<Player> players;
	int next_to_act = 0; // index of player next to act.

	// Game state
	double pot = 0.0;
	vector<double> bets_placed; // money put into the pot in the current round by
	// each player.
	vector<bool> actioned; // whether or not the player has made their required
	// action this round. folded means they have actioned.
	// when action is reopened, players still in the hand
	// have actioned = false. (action is re-requested).

	int previous_aggressor = -1; // has a raise (aggression) occured this round?
	// idx of the previous agressor.

	// public:
	unordered_map<HandAction, Node*> children;

	Node* parent = nullptr;

	// smells like bad design but i'll fix it later
	Node(Node* parent) {
		this->board1 = parent->board1;
		this->board2 = parent->board2;

		//this->actions = parent->actions;

		this->deck = parent->deck;
		this->players = parent->players;
		this->next_to_act = parent->next_to_act;
		this->pot = parent->pot;

		this->bets_placed = parent->bets_placed;
		this->actioned = parent->actioned;

		this->previous_aggressor = parent->previous_aggressor;

		this->parent = parent;
	}

	Node(const vector<int>& board1, const vector<int>& board2, int num_players, double stack_depth, double ante) {
		this->board1 = board1;
		this->board2 = board2;
		deck.erase(board1);
		deck.erase(board2);
		swap_boards_if_necessary();

		for (int i = 0; i < num_players; i++) {
			add_player(stack_depth, ante);
		}
	}

	void add_player(double stack_depth, double ante) {
		players.push_back(Player(deck.deal_with_modification(4), stack_depth - ante));
		pot += ante;

		bets_placed.push_back(0.0);
		actioned.push_back(false);
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

	void swap_boards_if_necessary() {
		if (this->board1[0] > this->board2[0]) {
			swap(this->board1, this->board2);
		}
	}

	// Calculates the correct filename for a given board1, board2, and actions
	// Preconditions: board1, board2, and actions are already set.
	// Syntax: CCCCC_CCCCC_AAAAAA.txt
	// C = card, like Ah = Ace of hearts.
	// A = action, enum in 0-9.
	// e.g. AhAdAc_2h3h4s_00110.txt
	string get_filename() {
		swap_boards_if_necessary();
		string filename = "";

		for (const auto& card : board1) {
			filename += phevaluator::Card(card).describeCard();
		}

		filename += "_";
		for (const auto& card : board2) {
			filename += phevaluator::Card(card).describeCard();
		}

		filename += "_";

		for (const auto& action : actions) {
			filename += string(1, action + '0');
		}

		filename += ".txt";

		return filename;
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

		// Debugging.
		if (debug) {
			cout << "Board1 = " << hand_to_string(board1) << endl;
			cout << "Board2 = " << hand_to_string(board2) << endl;
			cout << "Actions: " << endl;
			for (const HandAction& action : actions) {
				cout << action << " ";
			}
			cout << endl;

			cout << "Equities: " << endl;
			for (const auto& ev : evs) {
				cout << ev << " ";
			}
			cout << endl;
		}

		return evs;
	}

	void print_strategy(const vector<int> hand, const vector<pair<HandAction, double>>& strat) {

		cout << "Hand: " << hand_to_string(hand) << endl;

		cout << "Actions: ";
		for (const HandAction& action : actions) {
			cout << action << " ";
		}
		cout << endl;

		cout << std::fixed << std::setprecision(3);
		for (const auto& [action, probability] : strat) {
			cout << action << " " << probability << " ";
		}
		cout << endl;
	}

	// Adjust the strategy.
	// action_ev is the ev of various actions, performed by player_idx at this
	// node.
	void adjust_strategy(unordered_map<HandAction, double>& action_ev, int player_idx) {
		vector<int> hand = players[player_idx].get_hand();

		int handhash = hand_hash(hand);

		vector<pair<HandAction, double>> strat = get_strategy(player_idx);

		// weighted ev of this strategy.
		double strategy_ev = 0.0;
		for (const auto& [action, probability] : strat) {
			strategy_ev += probability * action_ev[action];
		}

		double total_positive_regret = 0.0;
		unordered_map<HandAction, double> positive_regrets;
		// Update regrets for each action
		for (auto& [action, probability] : strat) {
			double regret = action_ev[action] - strategy_ev; // CFR Regret formula

			if (regret > 0) {
				total_positive_regret += regret;
				positive_regrets[action] += regret;
			}
		}

		if (total_positive_regret > 0) {
			// Update strat
			vector<pair<HandAction, double>> strat = get_strategy(player_idx);

			for (int i = 0; i < strat.size(); i++) {
				HandAction action = strat[i].first;

				// if regret for action = 0, then probability will be 0
				strat[i] = { action, positive_regrets[action] / total_positive_regret };
			}

			strategy[handhash] = strat;
			// print_strategy(hand, strat);
		}
		else {
			// don't change strategy
			// should we fallback to default, or maintain previous?
			//
			//
			strategy[handhash] = get_default_strategy(player_idx);
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

	// Default strategy for allowable actions at a decision point.
	vector<pair<HandAction, double>> get_default_strategy(int player_idx) {
		Player player = players[player_idx];

		if (player.is_folded() || player.is_all_in()) {
			return { {NOTHING, 1.0} };
		}

		vector<HandAction> available;

		// pot/repot (same thing).
		// player can reraise as long as they can afford more than just calling.
		if (player.get_money() >= calculate_call(player_idx)) {
			available.push_back(POT);
		}

		if (previous_aggressor == -1) {
			available.push_back(CHECK);
		}
		else {
			available.push_back(FOLD);
			available.push_back(CALL);
		}

		vector<pair<HandAction, double>> strat;

		for (const auto& action : available) {
			strat.push_back({ action, 1.0 / (double)available.size() });
		}

		return strat;
	}

	vector<pair<HandAction, double>> get_strategy(int player_idx) {
		int handhash = hand_hash(players[player_idx].get_hand());

		// Return found strategy
		if (strategy.find(handhash) != strategy.end()) {
			return strategy[handhash];
		}

		// Not found - fallback to default.
		vector<pair<HandAction, double>> strat = get_default_strategy(player_idx);

		strategy[handhash] = strat;
		return strat;
	}

	HandAction get_next_action(const vector<pair<HandAction, double>> strat) {
		double chosen = rand_double(0.0, 1.0);

		double cumulative = 0.0;

		for (const auto& [action, probability] : strat) {
			cumulative += probability;

			if (chosen <= cumulative) {
				return action;
			}
		}

		return strat.back().first;
	}

	// Calculates, then performs next action, modifying this node.
	// Returns action performed.
	HandAction do_next_action() {
		Player& player = players[next_to_act]; // MUST BE A REFERENCE
		vector<pair<HandAction, double>> strat = get_strategy(next_to_act);

		HandAction action = get_next_action(strat);

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
		actions.push_back(action);
		return action;
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

	int get_next_to_act() { return next_to_act; }

	// Returns position on table like UTG, BTN
	// only works for 6-handed right now
	string GetTablePosition() const {
		static constexpr std::array<const char*, 6> positions = {
			"SB",    // 0  
			"BB",    // 1  
			"UTG",   // 2
			"HJ",    // 3  
			"CO",    // 4  
			"BTN",   // 5 

		};

		assert(next_to_act >= 0 && next_to_act < 6 && "next_to_act out of range");
		return positions[next_to_act];
	}
};

#endif