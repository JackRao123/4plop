// node.h
#pragma once 
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
#include <mutex>

#define MAX_UNIQUE_HANDS 52 * 52 * 52 * 52

using namespace std;
class Node {
	// Define a node based on
	// 1. Board
	// 2. Action sequence

public:
	// Strategy for each hand.
	// Define the strategy as the strategy for the person that action is on currently.
	// Maps handhash to strategy. each pair<HandAction, double> represents the probability at which we do that action.
	unordered_map<int, vector<pair<HandAction, double>>> strategy;

	//// Number of times each handhash has had its strategy computed.
	//unordered_map<int, int> num_strategy_computes_;

	// mutex for the strategy
	mutex mtx;

	// Cumulative positive regret for each info set (handhash -> action -> regret)
	unordered_map<int, unordered_map<HandAction, double>> cumulative_regret_;

	// Cumulative strategy sums for averaging (handhash -> action -> sum of prob_choice*reach)
	unordered_map<int, unordered_map<HandAction, double>> cumulative_strategy_;

	// How often you visited this info set (handhash -> total reach probability)
	unordered_map<int, double> visit_count_;




	// Traversal
	unordered_map<HandAction, Node*> children;
	Node* parent = nullptr;

	// Game state. Should be copied (and then modified) when spawning children. 
	GameState state_;





	Node() {}
	Node(const vector<int>& board1, const vector<int>& board2, int num_players, double stack_depth, double ante) {
		state_ = GameState(board1, board2, num_players, stack_depth, ante);
	}

	virtual ~Node() = default;

	void AdjustStrategy(unordered_map<HandAction, double>& action_ev, int player_idx, double reach_probability);


	// Default strategy for allowable actions at a decision point.
	// Returns a strategy where each allowed action has equal probability
	vector<pair<HandAction, double>> GetUniformStrategy(int player_idx);

	vector<pair<HandAction, double>> GetStrategy(int player_idx);


	// Randomises next action based on strategy probabilities.
	// Doesn't perform the action.
	// Returns {action to be performed, probability of choosing this action}.
	pair<HandAction, double> GetNextAction();

	// Creates and returns a child node, which has an action applied on it.
	Node* GetChild(HandAction action);

	// Returns position on table like UTG, BTN
	// only works for 6-handed right now
	string GetTablePosition() const;
};

