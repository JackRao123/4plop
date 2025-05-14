
#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <thread>

#include "src/simulation.h"

TEST(Profiling, Recurse) {
  Simulation sim;

  string flop1 = "AcKc8h";
  string flop2 = "KhQc4s";

  int num_players = 6;
  double stack_depth = 50.0;
  double ante = 5.0;

  sim.initialise(flop1, flop2, num_players, stack_depth, ante);
  sim.StartSolver();
  this_thread::sleep_for(chrono::seconds(5));
  sim.StopSolver();
}
