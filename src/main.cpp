#include "deck.h"
#include "equity_calc.h"
#include "gui.h"
#include "helper.h"
#include "include/phevaluator.h"
#include "node.h"
#include "simulation.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <unordered_set>
#include <vector>

using namespace std;

int main() {

  View view;
  view.Start();

  return 0;
}

// int main() {
//   Deck d;

//   vector<int> flop1 = string_to_hand("JcQdKc");
//   vector<int> flop2 = string_to_hand("8s2h5s");
//   vector<int> hand = string_to_hand("ThAh3s4s");

//   d.erase(flop1);
//   d.erase(flop2);
//   d.erase(hand);

//   Simulation s;
//   s.simulate(flop1, flop2, 6, 50.0, 5.0);

//   return 0;
// }
