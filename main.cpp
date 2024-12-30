#include <iostream>
#include "include/phevaluator.h"

using namespace std;


int main() {
  
	phevaluator::Rank r = phevaluator::EvaluatePlo4Cards(1, 2, 3, 4, 5, 6, 7, 8, 9);

	std::cout << "Rank is " << r.value() << std::endl;



  

  
  return 0;
}
