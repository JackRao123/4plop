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

// Ranks 2,3...K,A
// Suits c,d,h,s
// ID is rank*4 + suit

// When evaluated, we get a rank. Lower ranks are better.

class Deck {
private:
  vector<int> cards;
  mt19937 gen;

public:
  Deck() {
    for (int i = 0; i < 52; i++) {
      cards.push_back(i);
    }

    unsigned long long seed =
        chrono::system_clock::now().time_since_epoch().count();
    shuffle(cards.begin(), cards.end(), default_random_engine(seed));

    random_device rd;
    gen = mt19937(rd());
  }

  // deal deals a singular card without replacement
  // Param: none
  // Return: the index of the card.
  int deal() {
    if (cards.size() == 0) {
      throw std::runtime_error("ERROR: Tried to deal from empty deck.\n");
    }

    // no need to randomise since the deck is already shuffled.
    int card = cards.back();
    cards.pop_back();
    return card;
  }

  // deal_with_replacement deals num_cards cards with replacement.
  // Param:
  // int num_cards : number of cards to deal
  // Selects cards randomly each time to ensure subsequent calls are different.
  vector<int> deal_with_replacement(const int num_cards) {

    if (num_cards > cards.size()) {
      throw runtime_error("ERROR: Not enough cards\n");
    }

    uniform_int_distribution<int> dist(0, cards.size() - 1);
    unordered_set<int> chosen_cards;

    while (chosen_cards.size() != num_cards) {
      chosen_cards.insert(dist(gen));
    }

    vector<int> result(chosen_cards.begin(), chosen_cards.end());

    return result;
  }

  // deal_without_replacement deals num_cards cards without replacement.
  // Param:
  // int num_cards : number of cards to deal
  // takes the last num_cards cards, without reshuffling.
  vector<int> deal_without_replacement(const int num_cards) {

    if (num_cards > cards.size()) {
      throw runtime_error("ERROR: Not enough cards\n");
    }

    auto it = cards.end();
    advance(it, -num_cards);
    vector<int> chosen_cards(it, cards.end());
    cards.erase(it, cards.end());

    return chosen_cards;
  }
};

string hand_to_string(const vector<int> &hand) {
  string s;

  for (const auto &h : hand) {
    s += phevaluator::Card(h).describeCard();
  }

  return s;
}

int equity_calc() {
  Deck d;

  vector<int> h1 = d.deal_without_replacement(4);
  vector<int> h2 = d.deal_without_replacement(4);

  double sum_eq1 = 0.0;
  double sum_eq2 = 0.0;

  int num_iterations = 10000;

  int chop1 = 0;
  int chop2 = 0;
  int chopboth = 0;

  for (int i = 0; i < num_iterations; i++) {
    // b = both boards. First 5 is first board, second 5 is second board.
    vector<int> b = d.deal_with_replacement(10);

    phevaluator::Rank h1b1 = phevaluator::EvaluatePlo4Cards(
        b[0], b[1], b[2], b[3], b[4], h1[0], h1[1], h1[2], h1[3]);
    phevaluator::Rank h1b2 = phevaluator::EvaluatePlo4Cards(
        b[5], b[6], b[7], b[8], b[9], h1[0], h1[1], h1[2], h1[3]);

    phevaluator::Rank h2b1 = phevaluator::EvaluatePlo4Cards(
        b[0], b[1], b[2], b[3], b[4], h2[0], h2[1], h2[2], h2[3]);
    phevaluator::Rank h2b2 = phevaluator::EvaluatePlo4Cards(
        b[5], b[6], b[7], b[8], b[9], h2[0], h2[1], h2[2], h2[3]);

    // equity of player 1 and player 2. Sum to 1.00
    double eq1 = 0.0;
    double eq2 = 0.0;

    // First board
    if (h1b1 == h2b1) {
      // Chop
      eq1 += 0.25;
      eq2 += 0.25;
      chop1++;
    } else if (h1b1 > h2b1) {
      eq2 += 0.5;
    } else {
      eq1 += 0.5;
    }

    if (h1b2 == h2b2) {
      // Chop
      eq1 += 0.25;
      eq2 += 0.25;
      chop2++;
    } else if (h1b2 > h2b2) {
      eq2 += 0.5;
    } else {
      eq1 += 0.5;
    }

    if (eq1 == eq2) {
      chopboth++;
    }
    sum_eq1 += eq1;
    sum_eq2 += eq2;
  }

  cout << hand_to_string(h1) << " has equity "
       << sum_eq1 / (double)num_iterations << endl;

  cout << hand_to_string(h2) << " has equity "
       << sum_eq2 / (double)num_iterations << endl;

  cout << "Chop board 1 " << chop1 << "/" << num_iterations << " times "
       << endl;

  cout << "Chop board 2 " << chop2 << "/" << num_iterations << " times "
       << endl;

  cout << "Chop both boards " << chopboth << "/" << num_iterations << " times "
       << endl;

  return 0;
}

int main() {

  //   phevaluator::Rank r =
  //       phevaluator::EvaluatePlo4Cards(1, 2, 3, 4, 5, 6, 7, 8, 9);

  //   std::cout << "Rank is " << r.value() << std::endl;

  //   for (int i = 0; i < 52; i++) {
  //     cout << "Card " << i << " corresponds to "
  //          << phevaluator::Card(i).describeCard() << endl;
  //   }

  equity_calc();

  return 0;
}
