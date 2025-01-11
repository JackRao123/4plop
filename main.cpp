#include "cfr.h"
#include "deck.h"
#include "equity_calc.h"
#include "helper.h"
#include "include/phevaluator.h"
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

  //   phevaluator::Rank r =
  //       phevaluator::EvaluatePlo4Cards(1, 2, 3, 4, 5, 6, 7, 8, 9);

  //   std::cout << "Rank is " << r.value() << std::endl;

  //   for (int i = 0; i < 52; i++) {
  //     cout << "Card " << i << " corresponds to "
  //          << phevaluator::Card(i).describeCard() << endl;
  //   }

  Deck d;

  vector<int> flop1 = string_to_hand("JcQdKc");
  vector<int> flop2 = string_to_hand("8s2h5s");
  vector<int> hand = string_to_hand("8d8cKhKs");

  d.erase(flop1);
  d.erase(flop2);
  d.erase(hand);

  // multiway_equity_calc(hand, flop1, flop2, d);

  Simulation s;

  for (int i = 0; i < 1000; i++) {
	  s.simulate(flop1, flop2, 6, 200.0, 5.0);
  }

  //   vector<int> multiway_equity_calc();

  return 0;
}