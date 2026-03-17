#include "MainScene.hpp"
#include "Config.hpp"
#include "GLFW/glfw3.h"
#include "imgui.h"

#include <glm/gtc/matrix_transform.hpp>

MainScene::MainScene()
    : soundManager(SimpleSoundManager::Instance()),
      m_sushi_texture(Texture("./res/textures/sushi.png")),
      m_shark_texture(Texture("./res/textures/shark.png")),
      m_platform_texture(Texture("./res/textures/platform.png")),
      m_rock_texture(Texture("./res/textures/rock.png")),
      m_death_texture(Texture("./res/textures/death_text.png")) {

  m_player.position = glm::vec2(0.0f, 0.3f);
  m_player.velocity = glm::vec2(0.0f, 0.0f);
  m_player.jump_power = 4.5f;
  m_player.run_speed = 2.0f;
  m_player.size = glm::vec2(0.08f, 0.06f);
  m_player.isAlive = true;

  updateLocations();

  m_player_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_player_sprite.texture = &m_sushi_texture;

  m_shark_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_shark_sprite.texture = &m_shark_texture;

  Platform ground;
  ground.position = glm::vec2(0.0f, -0.5f);
  ground.size = glm::vec2(10.0f, 1.0f);
  m_platforms.push_back(ground);
  m_player.currentPlatform = &ground;

  generatePlatforms(m_player.position.y);

  m_death_sprite.position = glm::vec3(0.0f, 0.0f, 0.1f);
  m_death_sprite.size = glm::vec2(192.0f / 108.0f, 1.0f);
  m_death_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  m_death_sprite.texture = &m_death_texture;
}

MainScene::~MainScene() {}

void MainScene::onEnter() {
  m_config = LoadConfig();

  soundManager.Initialize();

  std::string music = "music";
  if (!soundManager.IsSoundPlaying(music)) {
    soundManager.LoadSound(music, "./res/sounds/drum.ogg");
    soundManager.PlayBackgroundMusic(music, m_config.volume);
  }
}

void MainScene::onExit() {}

Scene *MainScene::update(float delta) {
  updatePlayerPhysics(m_player, delta);

  updateLocations();

  generatePlatforms(m_player.position.y);

  m_fail_y += delta * m_fail_speed;
  m_fail_speed += m_fail_increase;

  if (m_restart)
    return new MainScene();
  return nullptr;
}

void MainScene::render(GLFWwindow &window, Renderer &renderer) {
  renderer.Clear(0.2f, 0.2f, 0.2f, 1.0f);

  glm::vec3 campos = m_camera.GetLocation();

  int w, h;
  glfwGetWindowSize(&window, &w, &h);
  glm::mat4 view = m_camera.GetViewMat();
  glm::mat4 projection = m_camera.GetProjectionMat(w, h);

  // walls
  renderer.BeginBatchDraw(2);
  SpriteInstance wall_left;
  wall_left.position = glm::vec3(-192 / 108 - 0.08f, 0.0f, 0.0f);
  wall_left.size = glm::vec2(0.1f, 10.0f);
  wall_left.color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
  wall_left.texture = &m_rock_texture;
  SpriteInstance wall_right;
  wall_right.position = glm::vec3(192 / 108 + 0.08f, 0.0f, 0.0f);
  wall_right.size = glm::vec2(0.1f, 10.0f);
  wall_right.color = glm::vec4(0.3f, 0.3f, 0.3f, 1.0f);
  wall_right.texture = &m_rock_texture;
  renderer.SubmitSprite(wall_left);
  renderer.SubmitSprite(wall_right);
  renderer.RendBatch(view, projection);

  // platforms
  renderer.BeginBatchDraw(m_platforms.size());
  for (const auto &p : m_platforms) {
    SpriteInstance plat_sprite;
    plat_sprite.position = glm::vec3(p.position, 0.0f);
    plat_sprite.size = p.size;
    plat_sprite.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    plat_sprite.texture = &m_platform_texture;
    renderer.SubmitSprite(plat_sprite);
  }
  renderer.RendBatch(view, projection);

  // player
  renderer.BeginBatchDraw(1);
  renderer.SubmitSprite(m_player_sprite);
  glm::mat4 model = glm::mat4(1.0f);
  float angle = -m_player.position.x * 5;
  glm::vec3 axis(0.0f, 0.0f, 1.0f);
  glm::mat4 rotation = glm::rotate(model, angle, axis);
  renderer.RendBatch(rotation, view, projection);

  // shark
  renderer.BeginBatchDraw(1);
  renderer.SubmitSprite(m_shark_sprite);
  model = glm::mat4(1.0f);
  rotation = glm::rotate(model, glm::half_pi<float>(), axis);
  renderer.RendBatch(rotation, view, projection);

  ////debug fail line
  // renderer.BeginBatchDraw(1);
  // SpriteInstance fail_line;
  // fail_line.position = glm::vec3(0.0f, m_fail_y, -0.01f);
  // fail_line.size = glm::vec2(10.0f, 0.01f);
  // fail_line.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
  // fail_line.texture = &m_rock_texture;
  // renderer.SubmitSprite(fail_line);
  // renderer.RendBatch(view, projection);

  if (!m_player.isAlive) {
    renderer.BeginBatchDraw(1);
    renderer.SubmitSprite(m_death_sprite);
    renderer.RendBatch(view, projection);
  }

  renderImgui(window, w, h);
}

void MainScene::handleInput(GLFWwindow &window, float delta) {
  GLCall(glfwPollEvents());

  handlePlayerInput(m_player, window, delta);

  if (glfwGetKey(&window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(&window, true);
}

void MainScene::updateLocations() {

  m_player_sprite.position = glm::vec3(m_player.position, 0.001f);
  m_player_sprite.size = m_player.size;
  // m_camera.SetCamera(m_player_sprite.position.x, m_player_sprite.position.y,
  // m_player_sprite.position.z - 1);
  m_camera.SetCamera(0.0f, m_player_sprite.position.y,
                     m_player_sprite.position.z - 1);

  m_shark_sprite.size = glm::vec2(1920.0f / 1080.0f, 1.0f);
  m_shark_sprite.position =
      glm::vec3(0.0f, m_fail_y - m_shark_sprite.size.x * 0.5, 0.002f);

  m_death_sprite.position =
      glm::vec3(0.0f, m_player_sprite.position.y, m_death_sprite.position.z);
}

void MainScene::renderImgui(GLFWwindow &window, int w, int h) {

  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_MenuBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoNav;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
  window_flags |= ImGuiWindowFlags_UnsavedDocument;
  window_flags |= ImGuiWindowFlags_NoBackground;

  float highscore_w = 1000;
  float highscore_h = 400;

  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(highscore_w, highscore_h), ImGuiCond_Always);

  ImGui::Begin("high score", NULL, window_flags);
  ImGui::SetWindowFontScale(5.0);

  ImGui::SetCursorPosX(0.0f);
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
  ImGui::Text("High Score: %f", m_config.highscore);
  ImGui::Text("Height: %f", m_player.position.y);

  ImGui::PopStyleColor();
  ImGui::SetWindowFontScale(1.0);
  ImGui::End();

  window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_MenuBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoNav;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

  float options_w = 200;
  float options_h = 100;

  ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
  ImGui::SetNextWindowPos(ImVec2(w - options_w, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(options_w, options_h), ImGuiCond_Always);

  ImGui::Begin("options", NULL, window_flags);

  // fullscreen
  bool fullscreen_changed = ImGui::Checkbox("fullscreen", &m_config.fullscreen);
  if (fullscreen_changed)
    SetFullScreen(window, m_config.fullscreen);

  // volume
  bool volume_changed =
      ImGui::SliderFloat(volume_label.c_str(), &m_config.volume, 0.0f, 1.0f);
  if (volume_changed)
    soundManager.SetSoundVolume("music", m_config.volume);

  if (volume_changed || fullscreen_changed)
    SaveConfig(m_config);

  ImGui::End();
}

void MainScene::SetFullScreen(GLFWwindow &window, bool fullscreen) {

  GLFWmonitor *monitor = glfwGetWindowMonitor(&window);

  if (monitor == NULL) {
    // set fullscreen
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowMonitor(&window, glfwGetPrimaryMonitor(), 0, 0, mode->width,
                         mode->height, mode->refreshRate);
  } else {
    glfwSetWindowMonitor(&window, NULL, 0, 0, 1280, 720, 0);
  }
}
