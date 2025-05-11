#ifndef GUI_H
#define GUI_H

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

// --- MVC pattern ---

class Model {};

class Controller {};

class View {
private:
  GLFWwindow *window_ = nullptr;
  Model &model_;
  Controller &controller_;

public:
  // boilerplate
  View() : model_(Model()), controller_(Controller()) {}

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

    window_ = glfwCreateWindow(1280, 720, "Campsie Hotel Software", nullptr, nullptr);
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
  void View::drawUI() {
    // Fetch the main viewport (the entire GLFW window area)
    ImGuiViewport *viewport = ImGui::GetMainViewport();

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

    static char buf_board1[128] = "";
    ImGui::InputText("Board 1", buf_board1, IM_ARRAYSIZE(buf_board1));

    static char buf_board2[128] = "";
    ImGui::InputText("Board 2", buf_board2, IM_ARRAYSIZE(buf_board2));

    static char buf_hand[128] = "";
    ImGui::InputText("Hand", buf_hand, IM_ARRAYSIZE(buf_hand));

    if (ImGui::Button("Solve")) {
      cout << "solving " << buf_board1 << " " << buf_board2 << " " << buf_hand << endl;
    }

    // 12) Tree nodes
    if (ImGui::TreeNode("UTG")) {

      if (ImGui::TreeNode("Check")) {

        if (ImGui::TreeNode("HJ")) {
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }

      else if (ImGui::TreeNode("Pot")) {
        if (ImGui::TreeNode("HJ")) {
          ImGui::TreePop();
        }

        ImGui::TreePop();
      }

      ImGui::TreePop();
    }

    // 13) Tables (simple)
    if (ImGui::BeginTable("table1", 5)) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Hand:");

      ImGui::TableSetColumnIndex(1);
      ImGui::Text("Check: ");

      ImGui::TableSetColumnIndex(2);
      ImGui::Text("Pot: ");

      ImGui::TableSetColumnIndex(3);
      ImGui::Text("Fold: ");

      for (int row = 0; row < 10; row++) {
        ImGui::TableNextRow();
      }
      ImGui::EndTable();
    }

    ImGui::End();
  }

  void Mainloop() {
    while (!glfwWindowShouldClose(window_)) {
      glfwPollEvents();

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      drawUI();

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