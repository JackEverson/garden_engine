#pragma once

// #include "Scene.hpp"
#include "Audio.hpp"
#include "MainScene.hpp"
#include "gl_debug.hpp"

#include "Button.hpp"

#include "Engine.hpp"
// #include "Renderer.hpp"

static bool start_game = false;
static bool quit_game = false;

class IntroScene : public Scene {

public:
  bool left_click_before = false;

  SimpleSoundManager &soundManager;

  Camera m_camera;

  Texture button_texture_exit;
  Texture button_texture_start;
  Texture sushi_texture;

  SpriteInstance button_sprite_start;
  SpriteInstance button_sprite_exit;
  SpriteInstance sushi_sprite;

  Button button_quit;
  Button button_start;

  IntroScene()
      : soundManager(SimpleSoundManager::Instance()),
        button_texture_exit(Texture("./res/textures/exit.png")),
        button_texture_start(Texture("./res/textures/start.png")),
        sushi_texture(Texture("./res/textures/sushi.png")),
        button_quit(glm::vec3(0.0f, -0.1f, 0.0f), glm::vec2(0.2f, 0.1f)),
        button_start(glm::vec3(0.0f, 0.1f, 0.0f), glm::vec2(0.2f, 0.1f)) {
    button_sprite_start.position = button_start.m_worldPosition;
    button_sprite_start.size = button_start.m_size;
    button_sprite_start.color = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    button_sprite_start.texture = &button_texture_start;

    button_sprite_exit.position = button_quit.m_worldPosition;
    button_sprite_exit.size = button_quit.m_size;
    button_sprite_exit.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    button_sprite_exit.texture = &button_texture_exit;

    sushi_sprite.position = glm::vec3(-0.7f, 0.0f, -0.2f);
    sushi_sprite.size = glm::vec2(1.2f, 0.8f);
    sushi_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sushi_sprite.texture = &sushi_texture;
  }

  void onEnter() override {

    // soundManager.LoadSound("beep", "./res/sounds/beep.wav");
  }

  void onExit() override {}

  Scene *update(float delta) override {

    if (start_game) {
      return new MainScene;
    }

    return nullptr;
  }

  void render(GLFWwindow &window, Renderer &renderer) override {

    renderer.Clear(0.2f, 0.2f, 0.2f, 1.0f);

    glm::vec3 campos = m_camera.GetLocation();

    int w, h;
    glfwGetWindowSize(&window, &w, &h);
    glm::mat4 view = m_camera.GetViewMat();
    glm::mat4 projection = m_camera.GetProjectionMat(w, h);

    renderer.BeginBatchDraw(1);
    renderer.SubmitSprite(sushi_sprite);
    renderer.RendBatch(view, projection);

    renderer.BeginBatchDraw(1);
    renderer.SubmitSprite(button_sprite_start);
    renderer.RendBatch(view, projection);

    renderer.BeginBatchDraw(1);
    renderer.SubmitSprite(button_sprite_exit);
    renderer.RendBatch(view, projection);
  }

  void handleInput(GLFWwindow &window, float delta) override {
    GLCall(glfwPollEvents());

    bool left_click = false;
    if (!left_click_before &&
        glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
      left_click = true;
    }

    // set left click before
    if (glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
      left_click_before = true;
    else
      left_click_before = false;

    if (!left_click) {
      return;
    }

    int w, h;
    glfwGetWindowSize(&window, &w, &h);

    double mouseX, mouseY;
    glfwGetCursorPos(&window, &mouseX, &mouseY);

    auto view = m_camera.GetViewMat();
    auto proj = m_camera.GetProjectionMat(w, h);

    if (button_start.IsMouseOverButton(view, proj, glm::vec2(mouseX, mouseY), w,
                                       h)) {
      // soundManager.PlaySound("beep");
      start_game = true;
    }

    if (button_quit.IsMouseOverButton(view, proj, glm::vec2(mouseX, mouseY), w,
                                      h)) {
      // soundManager.PlaySound("beep");
      quit_game = true;
    }

    if (glfwGetKey(&window, GLFW_KEY_ESCAPE) == GLFW_PRESS || quit_game)
      glfwSetWindowShouldClose(&window, true);
  }
};
