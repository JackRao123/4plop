// node.cpp
#include "node.h"
#include "chancenode.h"
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
#include <cassert>

using namespace std;





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
void Node::adjust_strategy(unordered_map<HandAction, double>& action_ev, int player_idx) {
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
		num_strategy_computes_[handhash]++;
		//cout << "Default strat" << endl;
	}
}



// Default strategy for allowable actions at a decision point.
// Returns a strategy where each allowed action has equal probability
vector<pair<HandAction, double>> Node::get_default_strategy(int player_idx) {
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


vector<pair<HandAction, double>> Node::get_strategy(int player_idx) {
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
HandAction Node::GetNextAction() {
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
Node* Node::GetChild(HandAction action) {
	GameState next_state = state_;
	next_state.do_next_action(action);

	Node* child;
	if (children.find(action) != children.end()) {
		// no need to update state.
		// if the sequence of actions is the same, the state will be the same.
		child = children[action];
	}
	else {
		if (next_state.end_of_action()) {
			child = new ChanceNode(next_state);
		}
		else {
			child = new Node();
		}
		child->state_ = next_state;
		child->parent = this;
		children[action] = child;
	}

	return child;
}


// Returns position on table like UTG, BTN
// only works for 6-handed right now
string Node::GetTablePosition() const {
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



