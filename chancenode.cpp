// chancenode.cpp
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

using namespace std;


// GetChild deals out the next street, and then returns a decision node.
Node* ChanceNode::GetChild() {
	GameState next_state = state_;
	pair<int, int> dealt_cards = next_state.next_street();

	Node* child;
	if (next_.find(dealt_cards) != next_.end()) {
		// if the child already exists, its state should be the correct value
		child = next_[dealt_cards];
	}
	else {
		child = new Node();
		child->state_ = next_state;
		child->parent = this;
		next_[dealt_cards] = child;
	}

	return child;
}

Node* ChanceNode::GetNextNode(int next_top_card, int next_bottom_card) {
	return next_[{next_top_card, next_bottom_card}];
}