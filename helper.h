#ifndef HELPER_H
#define HELPER_H
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

string hand_to_string(const vector<int> &hand) {
  string s;

  for (const auto &h : hand) {
    s += phevaluator::Card(h).describeCard();
  }

  return s;
}
// Takes a string like Ac9h3d etc
vector<int> string_to_hand(string s) {

  vector<int> hand;
  if (s.size() % 2 != 0) {
    throw runtime_error("String length be divisible by 2.");
  }

  for (int i = 0; i < s.size(); i = i + 2) {
    string card = s.substr(i, 2);
    hand.push_back(phevaluator::Card(card));
  }
  return hand;
}

// Random double range [min, max]
double rand_double(double min, double max) {
  static thread_local std::mt19937 gen(
      std::random_device{}()); // Thread-local random engine
  std::uniform_real_distribution<double> dis(min, max); // Uniform distribution
  return dis(gen);
}

// hashes hand to a unique number.
// Hand should be length exactly 4.
// todo: more efficient hashing system.
int hand_hash(vector<int> hand) {

  // sort ascending order.
  sort(hand.begin(), hand.end());

  int hash =
      hand[0] + 52 * hand[1] + 52 * 52 * hand[2] + 52 * 52 * 52 * hand[3];

  return hash;
}

#endif