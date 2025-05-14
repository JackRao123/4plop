#ifndef SIMULATION_H
#define SIMULATION_H
#include "equity_calc.h"
#include "include/phevaluator.h"
#include "node.h"
#include "chancenode.h"
#include "player.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <thread>
#include <unordered_set>
#include <vector>

enum SimulationState { RUNNING, PAUSED, STOPPED };

class Simulation {
private:
	int num_players_;

	Node* root_ = nullptr; // root of the game tree (shouldn't change)
	Node* focus_ = nullptr; // node from which we are running computations and are viewing strategy for

	mutex mtx;

	SimulationState state_ = SimulationState::STOPPED;
	thread worker_thread_;

public:
	// this calculates the optimal strategy
	// parameters
	// flop1 - top board flop
	// flop2 - bottom board flop
	// num_players - number of players.
	// stack_depth - stack depth, in $.
	// ante - bomb pot ante, in $.
	Simulation() {}


	// Recursive DFS for CFR:
	// Params:
	// Node: node to recurse from.
	// reach_probability: probability of reaching node.
	// Returns:
	// Vector of EVs for each player.
	vector<double> recurse(Node* node, double reach_probability) {

		if (node->state_.end_of_game()) {
			// If it is terminal, it is not a decision node.
			// So therefore just return EVs here.
			return node->state_.calculate_ev();
		}

		if (auto chance_node = dynamic_cast<ChanceNode*>(node)) {
			// next_node is a decision node - after dealing cards.
			Node* next_node = chance_node->GetChild();
			return recurse(next_node, reach_probability);
		}

		// This returns the EVs for all players but we only calculate the regret for
		// hero here. The rest we just pass back.
		vector<double> average_ev(num_players_);
		int hero = node->state_.get_next_to_act();

		// Calculate regret for hero.
		unordered_map<HandAction, double> action_ev;
		unordered_map<HandAction, int> action_count;

		int num_simulations = 1;
		for (int i = 0; i < num_simulations; i++) {
			auto [next_action, action_probability] = node->GetNextAction();
			Node* next = node->GetChild(next_action);

			vector<double> sample_ev = recurse(next, reach_probability * action_probability);

			for (int j = 0; j < num_players_; j++) {
				average_ev[j] += sample_ev[j];
			}

			action_ev[next_action] += sample_ev[hero];
			action_count[next_action]++;
		}

		// Convert to average
		for (int i = 0; i < num_players_; i++) {
			average_ev[i] /= (double)num_simulations;
		}

		// Convert to average
		for (const auto& [action, count] : action_count) {
			action_ev[action] /= (double)count;
		}

		// This is the strategy for 'next_to_act', at the current NODE.
		node->AdjustStrategy(action_ev, hero, reach_probability);

		return average_ev;
	}

	// Entry point
	// void initialise(const vector<int> &flop1, const vector<int> &flop2, int num_players, double stack_depth,
	//                 double ante) {
	void initialise(const string& flop1, const string& flop2, int num_players, double stack_depth, double ante) {

		if (flop1.size() != 6) {
			throw exception("Flop 1 is not correctly specified.");
		}
		if (flop2.size() != 6) {
			throw exception("Flop 2 is not correctly specified.");
		}

		vector<int> flop1vec = string_to_hand(flop1);
		vector<int> flop2vec = string_to_hand(flop2);

		set<int> unique_cards;
		unique_cards.insert(flop1vec.begin(), flop1vec.end());
		unique_cards.insert(flop2vec.begin(), flop2vec.end());

		if (unique_cards.size() != 6) {
			throw exception("The two flops must have 6 unique cards.");
		}

		num_players_ = num_players;

		root_ = new Node(flop1vec, flop2vec, num_players, stack_depth, ante);
		focus_ = root_;

		// for (int i = 0; i < 10000; i++) {
		//   focus_->reset(); // re-deals everyone cards.
		//   recurse(head);
		//   cout << i << endl;
		// }

		// Deal out cards randomly.
		// Recurse.
		// If a particular node is terminal, then calculate the amount of money the
		// hand gets. Adjust regrets accordingly.

		// For all possible actions from head:
		// Recurse along those
		// If a particular node is terminal, then return the amount of money that
		// each hand gets.

		// cout << "OK" << endl;
		// cout << focus_->strategy.size() << endl;

		// // vector<pair<HandAction, double>> strat = head->strategy[hand_hash(string_to_hand("8d8cKhKs"))];
		// // vector<pair<HandAction, double>> strat = head->strategy[hand_hash(string_to_hand("ThAh3s4s"))];

		// for (const auto &[hand_hash, strat] : head->strategy) {
		//   cout << "Hand: " << hand_hash_to_string(hand_hash) << endl;
		//   for (const auto &[action, probability] : strat) {
		//     cout << "Action " << action << " taken with probability " << probability << endl;
		//   }
		// }
	}

	// you should run this in a separate thread.
	void SolverLoop() {
		int iterations = 0;
		cout << "Starting solver loop " << endl;
		// loop exits on StopSolver();
		unique_lock<mutex> lock(mtx, defer_lock);
		while (true) {
			lock.lock();
			if (state_ == SimulationState::STOPPED) {
				lock.unlock();
				break;
			}
			else if (state_ == SimulationState::PAUSED) {
				lock.unlock();
				this_thread::sleep_for(chrono::milliseconds(100));
				continue;
			}

			// drop the lock while running the computation
			iterations++;
			//cout << "Done " << iterations << " iterations " << endl;

			lock.unlock();
			// recurse only from the root.
			// add functionality later for switching recursion basepoint.
			root_->state_.reset();
			recurse(root_, 1.0);
			//focus_->state_.reset();
			//recurse(focus_, 1.0);
		}
	}

	void StartSolver() {
		cout << "Starting solver " << endl;
		ResumeSolver();
		worker_thread_ = thread(&Simulation::SolverLoop, this);
	}

	void ResumeSolver() {
		lock_guard<mutex> lock(mtx);
		state_ = SimulationState::RUNNING;
	}

	void PauseSolver() {
		lock_guard<mutex> lock(mtx);
		state_ = SimulationState::PAUSED;
	}

	void StopSolver() {
		cout << "stopping solver " << endl;
		{
			lock_guard<mutex> lock(mtx);
			state_ = SimulationState::STOPPED;
		}
		worker_thread_.join();
	}

	void SetFocus(Node* new_focus) {
		PauseSolver();
		cout << "Changed focus from " << focus_ << " to " << new_focus << endl;
		cout << "Position: " << new_focus->GetTablePosition() << endl;

		focus_ = new_focus;
		ResumeSolver();
	}

	// GetFocus returns the current focus of the game tree.
	// it is the node for which we are looking at strategy for.
	Node* GetFocus() {
		return focus_;
	}

	// GetRoot returns the root of the game tree. The root never changes.
	Node* GetRoot() { return root_; }
};

#endif