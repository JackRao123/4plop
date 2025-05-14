#ifndef GUI_H
#define GUI_H

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <utility>
#include <mutex>

#include "simulation.h"
#include <GLFW/glfw3.h>

// --- MVC pattern ---

// class Model {}; "Simulation" is out model

// class Controller {
// private:
//   Simulation &simulation_;

//   // GUI node ID, to solver Node*
//   unordered_map<int, Node *> gui_node_to_solver_node_;

// public:
//   Controller() : simulation_(Simulation()) {}

//   void Solve(const string &flop1, const string &flop2) {
//     int num_players = 6;
//     double stack_depth = 50.0;
//     double ante = 5.0;

//     vector<int> flop1vec = string_to_hand(flop1);
//     vector<int> flop2vec = string_to_hand(flop2);

//     simulation_.initialise(flop1vec, flop2vec, num_players, stack_depth, ante);
//     simulation_.StartSolver();
//   }

//   vector<HandAction> OnNodeClicked(int node_id) {
//     Node *solver_node = gui_node_to_solver_node_[node_id];
//     simulation_.ChangeRoot(solver_node);
//     vector<HandAction> actions = solver_node->actions;

//     return actions;
//   }

//   Node * GetNextNode()
// };

// controller is so redundant i find that i am just bouncing back n forth Controller and View.

class View {
private:
	GLFWwindow* window_ = nullptr;
	// Controller &controller_;
	Simulation simulation_;

	// GUI node ID, to solver Node*
	unordered_map<int, Node*> gui_node_to_solver_node_;

	unordered_map<int, vector<pair<int, HandAction>>> gui_node_children_;
	unordered_map<int, int> gui_node_parent_;

	// is the node in the dropdown open
	unordered_map<int, bool> gui_node_is_open_;
	int focused_node_ = 0; // by default, the root

	// error handling
	string last_error_message_ = "";

public:
	// boilerplate
	View() : simulation_(Simulation()) {}

	// clean up all the gui shit
	~View() {
		// this is gpt code idk how this works
		if (window_) {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			glfwDestroyWindow(window_);
			glfwTerminate();
		}
	}

	void InitWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // no user resizing

		window_ = glfwCreateWindow(1280, 720, "Rec Pounder 9000", nullptr, nullptr);
		glfwMakeContextCurrent(window_);
		glfwSwapInterval(1);
	}

	void InitImGUI() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForOpenGL(window_, true);
		ImGui_ImplOpenGL3_Init("#version 330");
	}

	void Start() {
		InitWindow();
		InitImGUI();
		Mainloop();
	}

	// UI code
	void Solve(const string& flop1, const string& flop2) {
		int num_players = 6;
		double stack_depth = 50.0;
		double ante = 5.0;

		simulation_.initialise(flop1, flop2, num_players, stack_depth, ante);
		simulation_.StartSolver();
	}


	// For a node, Open parents, close children
	void FocusNode(int node_id) {
		//close everything and we just open ones we want
		gui_node_is_open_.clear();

		gui_node_is_open_[node_id] = true;
		int cur = node_id;
		while (gui_node_parent_.find(cur) != gui_node_parent_.end()) {
			cur = gui_node_parent_[cur];
			gui_node_is_open_[cur] = true;
		}

		simulation_.SetFocus(gui_node_to_solver_node_[node_id]);
	}

	void DrawNode(int node_id, string label) {
		// this bool represents if ImGUI thinks the node is open or not.
		bool is_open = ImGui::TreeNodeEx(label.c_str());

		if (ImGui::IsItemClicked()) {
			FocusNode(node_id);
		}

		if (gui_node_is_open_[node_id] && is_open) {
			// draw children recursively if they exist
			if (!gui_node_children_[node_id].empty()) {
				for (const auto& [child_id, action] : gui_node_children_[node_id]) {
					string label = gui_node_to_solver_node_[child_id]->GetTablePosition() + ": " + to_string(action);

					DrawNode(child_id, label);
				}
			}
			else {
				// otherwise, this is a leaf node, and create new children (which are closed)
				Node* solver_node = gui_node_to_solver_node_[node_id];

				for (const auto& [action, child_node] : solver_node->children) {
					int new_node_id = gui_node_to_solver_node_.size();
					gui_node_to_solver_node_[new_node_id] = child_node;
					gui_node_parent_[new_node_id] = node_id;
					gui_node_children_[node_id].push_back({ new_node_id, action });

					string label = child_node->GetTablePosition() + ": " + to_string(action);
					DrawNode(new_node_id, label);
				}
			}

			ImGui::TreePop();
		}
		else if (is_open) {
			//imgui thinks its open but our logic doesn't recurse
			ImGui::TreePop();
		}

	}

	void DrawUI() {
		// Fetch the main viewport (the entire GLFW window area)
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		// 1) Position & size to exactly cover it
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		// 2) Disable all decoration so there’s no title bar, border, resize, move, etc.
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | // no title bar, resize, etc
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus; // optional: disables gamepad/keyboard focusing

		// 3) Begin a full-screen “invisible” window
		ImGui::Begin("##FullScreen", nullptr, flags);

		// --- Now just lay out your controls as if they were on the desktop UI ---
		ImGui::Text("4plop solver");
		ImGui::Separator();

		static char buf_flop1[128] = "AcKc8h";
		ImGui::InputText("Board 1", buf_flop1, IM_ARRAYSIZE(buf_flop1));

		static char buf_flop2[128] = "2s3s5h";
		ImGui::InputText("Board 2", buf_flop2, IM_ARRAYSIZE(buf_flop2));

		// static char buf_hand[128] = "";
		// ImGui::InputText("Hand", buf_hand, IM_ARRAYSIZE(buf_hand));

		if (ImGui::Button("Solve")) {
			string flop1_str(buf_flop1);
			string flop2_str(buf_flop2);

			try {
				Solve(flop1_str, flop2_str);
			}
			catch (const exception& e) {
				last_error_message_ = e.what();
			}
		}

		// display error messages
		if (!last_error_message_.empty()) {
			ImGui::OpenPopup("Error");
			ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Appearing);
			if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::TextWrapped("%s", last_error_message_.c_str());
				if (ImGui::Button("OK")) {
					last_error_message_.clear();
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		// This is the entire tree
		const int root_node_id = 0;
		Node* solver_root = simulation_.GetRoot();
		if (solver_root != nullptr) {
			gui_node_to_solver_node_[root_node_id] = solver_root;
			gui_node_is_open_[root_node_id] = true;

			// Draw strategy tree (choices of actions)
			DrawNode(root_node_id, "Start");

			// Draw strategy table (strategy for each hand)
			// look through the strategy of focused node, and populate.
			int strategy_max_rows = 50;
			int strategy_rows_displayed = 0;
			ImGui::Separator();

			Node* focus = simulation_.GetFocus();


			// !!!
			// reference, or directly iterating through focus->strategy is much faster
			// copying, or using a mutex, is quite slow.
			// fastest way is probably to use a mutex and iterate through the map (because we are only displaying like 50 at a time).
			unordered_map<int, vector<pair<HandAction, double>>>& node_strategy = focus->strategy;
			//{
			//	lock_guard<mutex> lock(focus->mtx);
			//	node_strategy = focus->strategy;
			//}



			ImGui::Text("Current node: %s, hands: %d", focus->GetTablePosition().c_str(), node_strategy.size());

			static char buf_search[128] = "";
			ImGui::InputText("Search", buf_search, IM_ARRAYSIZE(buf_search));
			string search_term = string(buf_search);

			if (ImGui::BeginTable("table1", 6)) {
				// Display the strategy for this node.

				for (const auto& [hand_hash, strat] : node_strategy) {
					// search filter.
					string hand_string = hand_hash_to_string(hand_hash);
					if (hand_string.find(search_term) == string::npos) {
						continue;
					}

					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Hand: %s", hand_string.c_str());

					unordered_map<HandAction, double> strategymap(strat.begin(), strat.end());

					ImGui::TableSetColumnIndex(1);
					ImGui::Text("Check/Call: %f", strategymap[HandAction::CHECK]);

					ImGui::TableSetColumnIndex(2);
					ImGui::Text("Pot: %f", strategymap[HandAction::POT]);

					ImGui::TableSetColumnIndex(3);
					ImGui::Text("Fold: %f", strategymap[HandAction::FOLD]);

					ImGui::TableSetColumnIndex(4);
					ImGui::Text("Nothing: %f", strategymap[HandAction::NOTHING]);

					ImGui::TableSetColumnIndex(5);
					ImGui::Text("Visits: %f", focus->visit_count_[hand_hash]);

					strategy_rows_displayed++;
					if (strategy_rows_displayed >= strategy_max_rows) {
						break;
					}
				}

				ImGui::EndTable();
			}
		}


		// FPS counter
		float fps = ImGui::GetIO().Framerate;
		ImGui::Text("FPS: %.1f", fps);

		ImGui::End();
	}

	void Mainloop() {
		while (!glfwWindowShouldClose(window_)) {
			glfwPollEvents();

			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			DrawUI();

			ImGui::Render();
			int w, h;
			glfwGetFramebufferSize(window_, &w, &h);
			glViewport(0, 0, w, h);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window_);
		}
	}
};

#endif