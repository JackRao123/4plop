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
	Node(const vector<int>& board1, const vector<int>& board2, int num_players, double stack_depth, double ante) {
		state_ = GameState(board1, board2, num_players, stack_depth, ante);
	}

	virtual ~Node() = default;

	void adjust_strategy(unordered_map<HandAction, double>& action_ev, int player_idx);


	// Default strategy for allowable actions at a decision point.
	// Returns a strategy where each allowed action has equal probability
	vector<pair<HandAction, double>> get_default_strategy(int player_idx);

	vector<pair<HandAction, double>> get_strategy(int player_idx);

	// Randomises next action based on strategy probabilities.
	// Doesn't perform the action.
	HandAction GetNextAction();

	// Creates and returns a child node, which has an action applied on it.
	Node* GetChild(HandAction action);

	// Returns position on table like UTG, BTN
	// only works for 6-handed right now
	string GetTablePosition() const;
};

