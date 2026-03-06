#include <chrono>
#include <cstdio>
#include <print>
#include <string>
#include <thread>

#include <glad/glad.h>

#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Engine.hpp"
#include "gl_debug.hpp"

GardenEngine::GardenEngine(std::string name, bool windowed, int win_width,
                           int win_height)
    : soundManager(SimpleSoundManager::Instance()) {
  std::println("Creating GardenEngine...");

  setupGlfwWindow(name, windowed, win_width, win_height);
  setupOpenGl();
  setupImGui();
  setupAudio();

  std::println("GardenEngine Created!");
}

GardenEngine::~GardenEngine() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  delete m_renderer;
  m_renderer = nullptr;

  glfwDestroyWindow(m_window);
  glfwTerminate();
  soundManager.Shutdown();
}

int GardenEngine::Start(std::unique_ptr<Scene> scene, float fps) {
  std::println("GardenEngine Starting...");

  glClearColor(0.1, 0.6f, 0.2f, 1.0f);
  int frame_time_limit_ms = (int)((1 / fps) * 1000);

  m_currentScene = std::move(scene);
  m_currentScene->onEnter();

  float delta = 1.0f / fps;
  auto frame_start = std::chrono::steady_clock::now();
  auto program_start_time = std::chrono::steady_clock::now();

  while (!glfwWindowShouldClose(m_window)) {

    delta = calculateDeltaTime(frame_start);
    frame_start = std::chrono::steady_clock::now();

    Scene *next_scene = m_currentScene->update(delta);
    if (next_scene) {
      m_currentScene->onExit();
      m_currentScene.reset(next_scene);
      m_currentScene->onEnter();
    }

    prepImgui();
    m_currentScene->handleInput(*m_window, delta);
    m_currentScene->render(*m_window, *m_renderer);

    // checkImgui(); debug test
    renderImgui();

    GLCall(glfwSwapBuffers(m_window));

    // finish and fps limit
    auto end_time = std::chrono::steady_clock::now();
    auto duration = end_time - frame_start;
    int ms =
        (int)std::chrono::duration_cast<std::chrono::milliseconds>(duration)
            .count();
    int waittime = frame_time_limit_ms - ms;
    if (waittime < frame_time_limit_ms * -0.5) {
      // std::println("WARNING, cannot keep up with set FPS. FPS currently at
      // {}", 1000 / ms);
    } else
      std::this_thread::sleep_for(std::chrono::milliseconds(waittime));
  }

  // glfwTerminate();
  std::println("GardenEngine Exited Successfully!");
  return 0;
}

void GardenEngine::setupGlfwWindow(std::string win_name, bool windowed,
                                   int win_width, int win_height) {
  std::println("Creating GLFW Window...");

  if (!glfwInit()) {
    std::println("Failed to Initialize GLFW");
    throw std::runtime_error("Failed to Initialize GLFW");
  };

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  int monitor_count;
  GLFWmonitor **_monitors = glfwGetMonitors(&monitor_count);
  if (!monitor_count) {
    std::println("GLFWmonitor found monitors to be null");
    throw std::runtime_error("GLFWMonitor found monitors to be null");
  }

  std::println("GLFWmonitors has found {} monitors", monitor_count);
  for (int i = 0; i < monitor_count; i++) {
    const GLFWvidmode *mode = glfwGetVideoMode(_monitors[i]);
    std::println("Monitor {} has resolution {} x {}", i, mode->width,
                 mode->height);
  }

  GLFWmonitor *pMonitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(pMonitor);
  // std::println("Primary monitor detected to be");

  // std::cout << "Primary monitor detected to be:" << pMonitor << std::endl;

  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

  if (!windowed) {
    //// fullscreenborderless window
    // glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // No borders or title bar
    m_window = glfwCreateWindow(mode->width, mode->height, win_name.c_str(),
                                NULL, NULL);

    // m_window = glfwCreateWindow(mode->width, mode->height, win_name.c_str(),
    // pMonitor, NULL);

    // glfwSetWindowPos(m_window, 0, 0);
    glfwSetWindowMonitor(m_window, pMonitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  } else {
    // set windowed
    m_window = glfwCreateWindow(1280, 720, win_name.c_str(), NULL, NULL);
  }

  // vsync on
  glfwSwapInterval(1);
  glfwSetWindowAspectRatio(m_window, 16, 9);
  // load icon
  // GLFWimage icon;
  // icon.pixels = Loadimgpngdotjpg();
  // icon.width = 64;
  // icon.height = 64;
  // glfwSetWindowIcon(window, 1, &icon);

  if (!m_window) {
    std::println("Failed to create GLFW window");
    glfwTerminate();
    throw std::runtime_error("Failed to create GLFW window");
  }

  glfwMakeContextCurrent(m_window);

  glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);

  std::println("GLFW Window Created Successfully!");
}

void GardenEngine::setupOpenGl() {
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::println("Failed to initialize OpenGL context");
    throw std::runtime_error(
        "Failed to initialize OpenGL context through gladLoadGLLoader");
  }

  m_renderer = new Renderer();

  int display_w, display_h;
  glfwGetFramebufferSize(m_window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
}

void GardenEngine::setupImGui() {
  std::println("Creating ImGui Context...");
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO io = ImGui::GetIO();
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
  // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImGui_ImplGlfw_InitForOpenGL(m_window, true);
  ImGui_ImplOpenGL3_Init();

  std::println("ImGui Context Created!");
}

void GardenEngine::setupAudio() {
  if (!soundManager.Initialize()) {
    std::cerr << "Failed to initialize sound manager!" << std::endl;
    throw std::runtime_error("Failed to initialize sound manager!");
  }
}

void GardenEngine::prepImgui() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void GardenEngine::checkImgui() {
  ImGui::ShowDemoWindow(); // Show demo window! :)
}

void GardenEngine::renderImgui() {

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

float GardenEngine::calculateDeltaTime(
    std::chrono::steady_clock::time_point &lastFrameStart) {

  auto end_time = std::chrono::steady_clock::now();
  auto duration = end_time - lastFrameStart;

  float in_seconds = static_cast<float>(duration.count()) / 1'000'000'000.0f;

  return in_seconds;

  // int ms =
  //	(int)std::chrono::duration_cast<std::chrono::milliseconds>(duration)
  //	.count();
  // int waittime = frame_time_limit_ms - ms;
  // if (waittime < frame_time_limit_ms * -0.5) {
  //	//std::println("WARNING, cannot keep up with set FPS. FPS currently at
  //{}", 1000 / ms);
  // }
  // else
  //	std::this_thread::sleep_for(std::chrono::milliseconds(waittime));
}
