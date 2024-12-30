#include <iostream>
#include <include/phevaluator.h>
#include <include/rank.h>

int main() {
  // Example usage of phevaluator
  std::cout << "Welcome to 4plop with phevaluator!" << std::endl;

  // Replace with actual function calls from phevaluator
  // Example:
  // evaluate_hand();

  phevaluator::Rank r = phevaluator::EvaluatePlo4Cards(1, 2,3,8,9,4,5,6,7);


  for(int i = 0; i<52; i++){
    std::cout << phevaluator::Card(i).describeCard() << std::endl;
  }
  std::cout << r.value() << std::endl;

  std::cout << r.describeRank() << std::endl;
  return 0;
}
