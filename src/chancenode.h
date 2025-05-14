// chancenode.h 
#pragma once 
#include "node.h"
#include "gamestate.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <unordered_set>
#include <vector>

using namespace std;


struct PairHash {
	size_t operator()(const std::pair<int, int>& p) const noexcept {
		// very simple hash function
		return (static_cast<size_t>(p.first) << 32)
			^ static_cast<size_t>(p.second);
	}
};
struct PairEq {
	bool operator()(const std::pair<int, int>& a,
		const std::pair<int, int>& b) const noexcept {
		return a.first == b.first && a.second == b.second;
	}
};


//ChanceNode allows us to handle turns and rivers
class ChanceNode : public Node {
private:
	unordered_map<pair<int, int>, Node*, PairHash, PairEq> next_;

	GameState state_;
public:
	ChanceNode(GameState state) :state_(state) {}

	// GetChild deals out the next street, and then returns a decision node.
	Node* GetChild();

	Node* GetNextNode(int next_top_card, int next_bottom_card);
};
