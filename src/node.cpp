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
void Node::AdjustStrategy(unordered_map<HandAction, double>& action_ev, int player_idx, double reach_probability) {
	vector<int> hand = state_.players_[player_idx].get_hand();
	int handhash = hand_hash(hand);

	vector<pair<HandAction, double>> strat = GetStrategy(player_idx);

	// weighted ev of this strategy.
	double strategy_ev = 0.0;
	for (const auto& [action, probability] : strat) {
		strategy_ev += probability * action_ev[action];
	}

	// Update regrets for each action
	for (auto& [action, probability] : strat) {
		double regret = action_ev[action] - strategy_ev; // CFR Regret formula
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
			strat.push_back({ action,p });
		}
		strategy[handhash] = strat;
	}
	else { // fall back to default
		strategy[handhash] = GetUniformStrategy(player_idx);
	}

	visit_count_[handhash] += reach_probability;
}



// Default strategy for allowable actions at a decision point.
// Returns a strategy where each allowed action has equal probability
vector<pair<HandAction, double>> Node::GetUniformStrategy(int player_idx) {
	Player player = state_.players_[player_idx];

	if (player.is_folded() || player.is_all_in()) {
		return { {NOTHING, 1.0} };
	}

	vector<HandAction> available;

	// pot/repot (same thing).
	// player can reraise as long as they can afford more than just calling.
	if (player.get_money() >= state_.calculate_call(player_idx)) {
		available.push_back(POT);
	}

	if (state_.previous_aggressor_ == -1) {
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


// GetStrategy finds the strategy for a particular player at this node, or sets it to the uniform strategy if it doesn't exist.
vector<pair<HandAction, double>> Node::GetStrategy(int player_idx) {
	// !!!
	//lock_guard<mutex> lock(mtx);
	int handhash = hand_hash(state_.players_[player_idx].get_hand());
	if (strategy.find(handhash) == strategy.end()) {
		strategy[handhash] = GetUniformStrategy(player_idx);
	}

	return strategy[handhash];
}

// Randomises next action based on strategy probabilities.
// Doesn't perform the action.
// Returns {action to be performed, probability of choosing this action}.
pair<HandAction, double> Node::GetNextAction() {
	int next_to_act = state_.get_next_to_act();
	vector<pair<HandAction, double>> strat = GetStrategy(next_to_act);

	double chosen = rand_double(0.0, 1.0);
	double cumulative = 0.0;
	for (const auto& [action, probability] : strat) {
		cumulative += probability;

		if (chosen <= cumulative) {
			return { action, probability };
		}
	}

	return strat.back();
}

// Creates and returns a child node, which has an action applied on it.
Node* Node::GetChild(HandAction action) {
	GameState next_state = state_;
	next_state.do_next_action(action);

	Node* child;
	if (children.find(action) != children.end()) {
		child = children[action];
	}
	else {
		if (next_state.end_of_action()) {
			child = new ChanceNode(next_state);
		}
		else {
			child = new Node();
		}
		child->parent = this;
		children[action] = child;
	}

	// have to update the state no matter what - 
	// even though action sequences are the same, we must change what cards the players have between runs.
	child->state_ = next_state;
	return child;
}

// Returns position on table like UTG, BTN
// only works for 6-handed right now
string Node::GetTablePosition() const {
	static constexpr std::array<const char*, 6> positions = { "SB","BB","UTG","HJ","CO","BTN" };
	int nta = state_.get_next_to_act();
	assert(nta >= 0 && nta < state_.num_players_ && "next_to_act out of range");
	return positions[nta];
}