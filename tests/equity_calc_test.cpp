#include "src/equity_calc.h"

#include <gtest/gtest.h>

#include "src/helper.h"

TEST(EquityCalcTest, Test1) {
  vector<vector<int>> hands;

  hands.push_back(string_to_cards("AcAdAsAh"));
  hands.push_back(string_to_cards("KcKdKsKh"));

  vector<int> board1 = string_to_cards("2c2d2s2h4h");
  vector<int> board2 = string_to_cards("3c3d3s3h5h");

  vector<double> equities = equity_calc(hands, board1, board2);

  ASSERT_EQ(equities[0], 1.0);
  ASSERT_EQ(equities[1], 0.0);
}
