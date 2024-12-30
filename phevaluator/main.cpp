#include <iostream>
#include <include/phevaluator.h>
#include <include/rank.h>

int main() {
  // Example usage of phevaluator
  std::cout << "Welcome to 4plop with phevaluator!" << std::endl;

  // Replace with actual function calls from phevaluator
  // Example:
  // evaluate_hand();

  phevaluator::Rank ev = phevaluator::EvaluatePlo4Cards(1, 2, 3, 4, 5, 6, 7, 8, 9);
  std::cout << ev.value() << std::endl;
  return 0;
}
