#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/phevaluator.h"

using namespace std;

// Ranks 2,3...K,A
// Suits c,d,h,s
// ID is rank*4 + suit

// When evaluated, we get a rank. Lower ranks are better.

// cards_to_string converts a vector of cards of any length to a string
// representation
inline string cards_to_string(const vector<int>& cards) {
  string s;

  for (const auto& c : cards) {
    s += phevaluator::Card(c).describeCard();
  }

  return s;
}

// string_to_cards converts a string representation of a set of cards of any
// length, to a vector for example AcAdKc -> {x,y,z}
inline vector<int> string_to_cards(string s) {
  vector<int> cards;
  if (s.size() % 2 != 0) {
    throw runtime_error("Card set string length be divisible by 2.");
  }

  for (int i = 0; i < s.size(); i = i + 2) {
    string card = s.substr(i, 2);
    cards.push_back(phevaluator::Card(card));
  }
  return cards;
}

// Random double range [min, max]
inline double rand_double(double min, double max) {
  static thread_local std::mt19937 gen(
      std::random_device{}());  // Thread-local random engine
  std::uniform_real_distribution<double> dis(min, max);  // Uniform distribution
  return dis(gen);
}

// hashes hand to a unique number.
// Hand should be length exactly 4.
// todo: more efficient hashing system.
inline int hand_hash(vector<int> hand) {
  // sort ascending order.
  sort(hand.begin(), hand.end());

  int hash =
      hand[0] + 52 * hand[1] + 52 * 52 * hand[2] + 52 * 52 * 52 * hand[3];

  return hash;
}

inline string hand_hash_to_string(int hash) {
  vector<int> hand;

  for (int i = 3; i >= 0; i--) {
    hand.push_back(hash / (int)(pow(52, i)));
    hash = hash % (int)pow(52, i);
  }

  return cards_to_string(hand);
}