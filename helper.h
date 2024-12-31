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
#endif