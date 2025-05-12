#ifndef NODE_H
#define NODE_H
#include "equity_calc.h"
#include "include/phevaluator.h"
#include "player.h"
#include "gamestate.h"
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

class Node {
	// Define a node based on
	// 1. Board
	// 2. Action sequence

public:
	// Strategy for each hand.
	// Define the strategy as the strategy for the next-to-act person.
	// Maps handhash to strategy. each pair<HandAction, double> represents the probability at which we do that action.
	unordered_map<int, vector<pair<HandAction, double>>> strategy;

	// Number of times each handhash has been computed.
	unordered_map<int, int> num_strategy_computes_;

	// ev of a particular hand at the current node.
	// evaluated only at terminal nodes.
	unordered_map<int, double> ev;

	// Traversal
	unordered_map<HandAction, Node*> children;
	Node* parent = nullptr;


	// Game state. Should be copied when spawning children. 
	GameState state_;

	Node() {}

	// smells like bad design but i'll fix it later
	Node(Node* parent) {
		state_ = parent->state_; // copy

		this->parent = parent;


		//state_.board1 = parent->board1;
		//state_.board2 = parent->board2;


		//this->deck = parent->deck;
		//this->players = parent->players;
		//this->next_to_act = parent->next_to_act;
		//this->pot = parent->pot;

		//this->bets_placed = parent->bets_placed;
		//this->actioned = parent->actioned;

		//this->previous_aggressor = parent->previous_aggressor;

		//this->parent = parent;
	}

	Node(const vector<int>& board1, const vector<int>& board2, int num_players, double stack_depth, double ante) {
		state_ = GameState(board1, board2, num_players, stack_depth, ante);
	}







	//// Calculates the correct filename for a given board1, board2, and actions
	//// Preconditions: board1, board2, and actions are already set.
	//// Syntax: CCCCC_CCCCC_AAAAAA.txt
	//// C = card, like Ah = Ace of hearts.
	//// A = action, enum in 0-9.
	//// e.g. AhAdAc_2h3h4s_00110.txt
	//string get_filename() {
	//	swap_boards_if_necessary();
	//	string filename = "";

	//	for (const auto& card : board1) {
	//		filename += phevaluator::Card(card).describeCard();
	//	}

	//	filename += "_";
	//	for (const auto& card : board2) {
	//		filename += phevaluator::Card(card).describeCard();
	//	}

	//	filename += "_";

	//	for (const auto& action : actions) {
	//		filename += string(1, action + '0');
	//	}

	//	filename += ".txt";

	//	return filename;
	//}






	//void print_strategy(const vector<int> hand, const vector<pair<HandAction, double>>& strat) {

	//	cout << "Hand: " << hand_to_string(hand) << endl;

	//	cout << "Actions: ";
	//	for (const HandAction& action : actions) {
	//		cout << action << " ";
	//	}
	//	cout << endl;

	//	cout << std::fixed << std::setprecision(3);
	//	for (const auto& [action, probability] : strat) {
	//		cout << action << " " << probability << " ";
	//	}
	//	cout << endl;
	//}

	// Adjust the strategy.
	// action_ev is the ev of various actions, performed by player_idx at this
	// node.
	void adjust_strategy(unordered_map<HandAction, double>& action_ev, int player_idx) {
		vector<int> hand = state_.players[player_idx].get_hand();

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
			num_strategy_computes_[handhash]++;
			// print_strategy(hand, strat);
		}
		else {
			// don't change strategy
			// should we fallback to default, or maintain previous?
			//strategy[handhash] = get_default_strategy(player_idx);
			//num_strategy_computes_[handhash]++;
			//cout << "Default strat" << endl;
		}
	}



	// Default strategy for allowable actions at a decision point.
	// Returns a strategy where each allowed action has equal probability
	vector<pair<HandAction, double>> get_default_strategy(int player_idx) {
		Player player = state_.players[player_idx];

		if (player.is_folded() || player.is_all_in()) {
			return { {NOTHING, 1.0} };
		}

		vector<HandAction> available;

		// pot/repot (same thing).
		// player can reraise as long as they can afford more than just calling.
		if (player.get_money() >= state_.calculate_call(player_idx)) {
			available.push_back(POT);
		}

		if (state_.previous_aggressor == -1) {
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
		int handhash = hand_hash(state_.players[player_idx].get_hand());

		// Return found strategy
		if (strategy.find(handhash) != strategy.end()) {
			return strategy[handhash];
		}

		// Not found - fallback to default.
		vector<pair<HandAction, double>> strat = get_default_strategy(player_idx);

		strategy[handhash] = strat;
		return strat;
	}



	// Randomises next action based on strategy probabilities.
	// Doesn't perform the action.
	HandAction GetNextAction() {
		int next_to_act = state_.get_next_to_act();
		vector<pair<HandAction, double>> strat = get_strategy(next_to_act);

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

	// Creates and returns a child node, which has an action applied on it.
	Node* GetChild(HandAction action) {
		Node* child;
		if (children.find(action) != children.end()) {
			child = children[action];
			child->state_ = state_; // overwrite state. 
		}
		else {
			child = new Node();
			child->state_ = state_;
			children[action] = child;
			child->parent = this;
		}

		child->state_.do_next_action(action);

		return child;
	}



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

		assert(state_.get_next_to_act() >= 0 && state_.get_next_to_act() < 6 && "next_to_act out of range");
		return positions[state_.get_next_to_act()];
	}
};


//class ChanceNode : public Node {
//private:
//	unordered_map<pair<int, int>, Node*> next_;
//
//public:
//
//	Node* GetNextNode(int next_top_card, int next_bottom_card) {
//		return next_[{next_top_card, next_bottom_card}];
//	}
//
//};


#endif