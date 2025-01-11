#include <gtest/gtest.h>
//gpt code (stub - testing cmake)
// 
// 
// Function to test
int add(int a, int b) {
    return a + b;
}

// Test cases
TEST(AdditionTest, PositiveNumbers) {
    EXPECT_EQ(add(1, 1), 2);  // 1 + 1 = 2
}

TEST(AdditionTest, NegativeNumbers) {
    EXPECT_EQ(add(-1, -1), -2);  // -1 + -1 = -2
}

TEST(AdditionTest, MixedNumbers) {
    EXPECT_EQ(add(1, -1), 0);  // 1 + (-1) = 0
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();  // Runs all the test cases
}
