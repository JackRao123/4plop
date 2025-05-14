#pragma once

#include <vector>

using namespace std;

class Player {
 public:
  vector<int> hand;
  double money;

  bool folded = false;

 public:
  Player(const vector<int>& hand, double money) {
    this->money = money;
    this->hand = hand;
  }

  void fold() { folded = true; }

  bool is_folded() { return folded; }

  bool is_all_in() { return money < 0.001; }

  double get_money() { return money; }

  void subtract_money(double money) { this->money -= money; }

  void add_money(double money) { this->money += money; }

  vector<int> get_hand() { return hand; }
};
