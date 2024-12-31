#include "deck.h"
#include "include/phevaluator.h"
#include "player.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

using namespace std;

// Goal: Find optimal multiway strategy
// Actions:
// 1. Check/fold
// 2. Call
// 3. Pot.
// Initialise all to equal (33%) probability.

// Regret(action) += U()

class Simulation {

private:
  double pot = 0.0;

  vector<Player> players;
  Deck deck;

public:
  Simulation(int num_players) {}
};
