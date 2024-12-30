#include <bits/stdc++.h>

using namespace std;

int suits[4] = {'d', 'c', 'h', 's'};
int ranks[13] = {'2', '3', '4', '5', '6', '7', '8',
                 '9', 'T', 'J', 'Q', 'K', 'A'};

inline int _rank(int card) { return card % 13; }
inline int _suit(int card) { return card / 13; }

string card_to_symbol(int card) {
  return string(1, ranks[_rank(card)]) + string(1, suits[_suit(card)]);
}

class Deck {
private:
  vector<int> cards;

  // Initialise with 52 cards
public:
  Deck() {
    for (int i = 0; i < 52; i++) {
      cards.push_back(i);
    }

    unsigned long long seed =
        chrono::system_clock::now().time_since_epoch().count();
    shuffle(cards.begin(), cards.end(), default_random_engine(seed));
  }

  // deal deals a singular card.
  // Param: none
  // Return: the index of the card.
  int deal() {

    if (cards.size() == 0) {
      throw std::runtime_error("ERROR: Tried to deal from empty deck.\n");
    }

    int card = cards.back();
    cards.pop_back();

    return card;
  }
};

// function to hash a hand to a value
// fall into buckets
// each has a distinct integer ordering

// It doesn't matter if there are gaps in between, because all we need is a
// strict ordering. All of these are simplified Essentially a hash function

// Hand classes
// High card - 13 ^ 5 combos  (5 x high cards)

// 1 pair - 13 * (13 ^ 3) (the rank of pair, then 3 remaining high cards)

// 2 pair - 13 * 13 * 13 (rank of first pair * rank of second pair * rank of
// high card)

// trips -  13 * 13 * 13 (rank of trips * rank of 4th high card * rank of 5th
// high card)

// set - 13 * 13 * 13 (rank of set * rank of 4th high card * rank of 5th high
// card)

// straight - 13 - there are 9 possible straights. simplify to 13.

// flush - 13c5 = 1287 possible flushes. Simplify to 13^5.

// boat - 13*13 - (rank of 3x and rank of 2x)

// quads - 13*13 - (rank of quads, and rank of high card)

// straight flush -  same as straight. 13.

// royal flush - 1. have it or you dont.

// Constants for hand bucket calculations
// Bucket sizes simplified. 13^5 is maximum bucket size.
// Hand ranking: THe higher the better.
// Each class of hand is in a separate bucket.
const int OFFSET_HIGH_CARD =
    0; // 13 * 13 * 13 * 13 * 13   High card combinations
const int OFFSET_PAIR =
    1e6; // 13 * (13 * 13 * 13)           One pair combinations
const int OFFSET_TWO_PAIR =
    2e6; // 13 * 13 * 13              Two pair combinations
const int OFFSET_THREE_OF_A_KIND =
    3e6; // 13 * 13 * 13       set/trips combinations
const int OFFSET_STRAIGHT =
    4e6; // 13                        straight combinations
const int OFFSET_FLUSH =
    5e6; // 13 * 13 * 13 * 13 * 13       Flushes combinations
const int OFFSET_FULL_HOUSE = 6e6;     // 13 * 13             Boat combinations
const int OFFSET_FOUR_OF_A_KIND = 7e6; // 13 * 13             Quads combinations
const int OFFSET_STRAIGHT_FLUSH =
    8e6;                            // 13                straight combinations
const int OFFSET_ROYAL_FLUSH = 9e6; // 1                   Royal flush

// Sorts cards by rank ascending. 2 is lowest, A is highest.
void sort_by_rank(vector<int> &cards) {
  sort(cards.begin(), cards.end(),
       [&](const int &a, const int &b) { return a % 13 < b % 13; });
}

int high_card(vector<int> cards) {
  sort_by_rank(cards);

  int score = 0;

  int pwr = 1;
  for (int i = 2; i < 7; i++) {
    score = score + pwr * _rank(cards[i]);
    pwr = pwr * 13;
  }

  return score;
}

int one_pair(vector<int> cards) {
  sort_by_rank(cards);

  int paired_card = -1;

  for (int i = 1; i < 7; i++) {
    if (_rank(cards[i]) == _rank(cards[i - 1])) {
      paired_card = _rank(cards[i]);
    }
  }

  if (paired_card == -1) {
    return 0;
  }

  int score = paired_card * 13 * 13 * 13;

  int pwr = 13 * 13;

  for (int i = 6; i >= 0 && pwr != 0; i--) {
    int rnk = _rank(cards[i]);

    if (rnk == paired_card) {
      continue;
    }

    score += rnk * pwr;

    pwr = pwr / 13;
  }

  return score;
}

int two_pair(vector<int> cards) {
  int pair_one = -1;
  int pair_two = -1;

  for (int i = 1; i < 7; i++) {
    if (_rank(cards[i]) == _rank(cards[i - 1])) {

      if (pair_one == -1) {
        pair_one = _rank(cards[i]);
      } else {
        pair_two = _rank(cards[i]);
      }
    }
  }

  if (pair_one == -1 || pair_two == -1) {
    return 0;
  }

  // let pair_one be the smaller pair.
  if (pair_one > pair_two) {
    swap(pair_one, pair_two);
  }

  int score = 13 * 13 * pair_two + 13 * pair_one + 0; // + high card.

  for (int i = 6; i >= 0; i--) {
    int rnk = _rank(cards[i]);

    if (rnk != pair_one && rnk != pair_two) {
      score += rnk;
      break;
    }
  }

  return score;
}

int main() {
  Deck d;

  for (int i = 0; i < 52; i++) {
    cout << card_to_symbol(d.deal()) << endl;
  }

  return 0;
}
