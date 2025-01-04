#ifndef DECK_H
#define DECK_H

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

class Deck {
public:
  vector<int> cards;
  mt19937 gen;

  Deck() {
    for (int i = 0; i < 52; i++) {
      cards.push_back(i);
    }

    shuffle();

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

  void shuffle() {
    unsigned long long seed =
        chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(cards.begin(), cards.end(), default_random_engine(seed));
  }
  // deal_without_modification deals num_cards cards without modifying deck.
  // Param:
  // int num_cards : number of cards to deal
  // Selects cards randomly each time to ensure subsequent calls are different.
  vector<int> deal_without_modification(const int num_cards) {

    if (num_cards > cards.size()) {
      throw runtime_error("ERROR: Not enough cards\n");
    }

    uniform_int_distribution<int> dist(0, cards.size() - 1);
    unordered_set<int> chosen_cards;

    while (chosen_cards.size() != num_cards) {
      chosen_cards.insert(cards[dist(gen)]);
    }

    vector<int> result(chosen_cards.begin(), chosen_cards.end());

    return result;
  }

  // deal_with_modification deals num_cards cards with modifying deck.
  // Param:
  // int num_cards : number of cards to deal
  // takes the last num_cards cards, without reshuffling.
  vector<int> deal_with_modification(const int num_cards) {

    if (num_cards > cards.size()) {
      throw runtime_error("ERROR: Not enough cards\n");
    }

    auto it = cards.end();
    advance(it, -num_cards);
    vector<int> chosen_cards(it, cards.end());
    cards.erase(it, cards.end());

    return chosen_cards;
  }

  // erase removes specific cards from the deck. Throws error if they are not
  // found. Param: vector<int> cards_to_remove: vector of cards to remove.
  // cards_to_remove must be unique
  //(you wont ever have to remove cards that are the same anyways)
  void erase(vector<int> cards_to_remove) {

    // This is O(n), but its fine. I don't think erase() will be called often
    // except for when initialising. If it is called

    int p1 = 0;
    int p2 = 0;

    unordered_map<int, bool> should_remove;

    for (const auto &c : cards_to_remove) {
      should_remove[c] = true;
    }

    int num_removed = 0;
    while (p2 < cards.size()) {
      if (should_remove[cards[p2]]) {
        num_removed++;
        p2++;
        continue;
      }

      cards[p1] = cards[p2];
      p1++;
      p2++;
    }

    auto it = cards.begin();
    advance(it, p1);
    cards.erase(it, cards.end());

    if (num_removed != cards_to_remove.size()) {
      throw runtime_error("Failed to remove some cards. Probably duplicate "
                          "card error somewhere.");
    }
  }

  void replace(int card) {
    cards.push_back(card);
    shuffle();
  }
};

#endif