
#include "Engine.hpp"

#include "ExitScene.hpp"
#include "GameLoopScene.hpp"

#include <glm/gtc/matrix_transform.hpp>

GameLoopScene::GameLoopScene()
    : soundManager(SimpleSoundManager::Instance()),
      m_approacher(Approacher(30.0f, 3.0f)),
      approacher_texture(Texture("./res/textures/sushi.png")),
      background_texture(Texture("./res/textures/background.png")),
      floor_texture(Texture("./res/textures/gravel_floor.png")),
      wall_texture(Texture("./res/textures/concrete_wall.png")),
      ceiling_texture(Texture("./res/textures/plaster_ceiling.png")),
      door_texture(Texture("./res/textures/metal_door.png")),
      rock_texture(Texture("./res/textures/rock.png")),
      key_texture(Texture("./res/textures/key.png")),
      delivered_text(Texture("./res/textures/delivered_text.png")),
      button_rock(glm::vec3(-0.4f, -0.45f, -1.0f), glm::vec2(0.1f, 0.1f)),
      button_key(glm::vec3(-0.4f, -0.45f, -1.0f), glm::vec2(0.1f, 0.1f)),
      button_lock(glm::vec3(0.5f, -0.05, -0.9f), glm::vec2(0.1f, 0.1f)) {}

GameLoopScene::~GameLoopScene() {}

void GameLoopScene::onEnter() {
  soundManager.LoadSound("beep", "./res/sounds/beep.wav");
  soundManager.LoadSound("jumpscare", "./res/sounds/ghost-scare-vintage.wav");

  soundManager.LoadSound("ambient", "./res/sounds/ambient-noise.ogg");
  soundManager.PlayBackgroundMusic("ambient", 1.0f);
}

void GameLoopScene::onExit() {}

Scene *GameLoopScene::update(float delta) {
  m_approacher.Step();

  if (m_approacher.m_distanceAway < 0.1f && !m_jump_scared) {
    m_jump_scared = true;
    soundManager.PlaySound("jumpscare");
  }

  if (m_approacher.m_distanceAway == 0) {
    m_death = true;
  }

  if (escaped) {
    m_approacher.stopped = true;
    static int counter = 0;
    counter++;
    if (counter > 50)
      return new ExitScene;
  }

  return nullptr;
}

void GameLoopScene::render(GLFWwindow &window, Renderer &renderer) {
  renderer.Clear(0.1f, 0.1f, 0.1f, 1.0f);

  float clicks = (float)m_clickCounter.GetClicks();

  glm::vec3 campos = m_camera.GetLocation();

  int w, h;
  glfwGetWindowSize(&window, &w, &h);
  // float aspect = w / h;
  glm::mat4 view = m_camera.GetViewMat();
  glm::mat4 projection = m_camera.GetProjectionMat(w, h);

  DrawHallway(view, projection, renderer);

  float jiggle_size_x = 0.01f;
  float jiggle_size_y = 0.001f;
  float jiggle_speed = 20.0f;

  float jiggle_x =
      glm::cos(m_approacher.m_distanceAway * jiggle_speed) * -jiggle_size_x;
  float jiggle_y =
      glm::sin(m_approacher.m_distanceAway * jiggle_speed) * jiggle_size_y;

  float darkness = 1.0f - 0.6f * (m_approacher.m_distanceAway / 30.0f);

  renderer.BeginBatchDraw(4);

  if (m_death || escaped) {
    SpriteInstance death_sprite;

    std::mt19937 rng(std::random_device{}()); // Mersenne Twister seeded with
                                              // hardware entropy
    std::uniform_int_distribution<int> dist(1, 10);
    int randomValueX = dist(rng);
    int randomValueY = dist(rng);
    float size_adjustX = randomValueX * 0.01f;
    float size_adjustY = randomValueY * 0.01f;

    death_sprite.position = glm::vec3(0.0f, 0.0f, -m_approacher.m_distanceAway);
    death_sprite.size = glm::vec2(0.6f + size_adjustX, 0.4f + size_adjustY);
    death_sprite.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    death_sprite.texture = &approacher_texture;
    renderer.SubmitSprite(death_sprite);
  } else {
    SpriteInstance approacher_sprite;
    approacher_sprite.position =
        glm::vec3(jiggle_x, jiggle_y, -m_approacher.m_distanceAway);
    approacher_sprite.size = glm::vec2(0.6f, 0.4f);
    approacher_sprite.color = glm::vec4(darkness, darkness, darkness, 1.0f);
    approacher_sprite.texture = &approacher_texture;
    renderer.SubmitSprite(approacher_sprite);
  }
  renderer.RendBatch(view, projection);

  if (m_approacher.m_distanceAway <= 0.0f) {
    renderer.BeginBatchDraw(1);
    SpriteInstance deliver_sprite;
    deliver_sprite.position = glm::vec3(0.0f, 0.0f, 0.01f);
    deliver_sprite.size = glm::vec2(1.0f, 1.0f);
    deliver_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    deliver_sprite.texture = &delivered_text;
    renderer.SubmitSprite(deliver_sprite);
    renderer.RendBatch(view, projection);
  }
}

void GameLoopScene::handleInput(GLFWwindow &window, float delta) {
  GLCall(glfwPollEvents());

  if (glfwGetKey(&window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(&window, true);

  double mouseX, mouseY;
  glfwGetCursorPos(&window, &mouseX, &mouseY);

  int w, h;
  glfwGetWindowSize(&window, &w, &h);
  float aspect_ratio = (float)w / (float)h;

  if (glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    // glm::vec3 cam_loc = m_camera.GetLocation();

    // double nxpos = (mouseX / w + cam_loc.x);
    // double nypos = (1 - (mouseY / h)) + cam_loc.y;

    if (m_first_click == true) {
      m_first_click = false;
      m_clickCounter.click();
      soundManager.PlaySound("beep");

      auto view = m_camera.GetViewMat();
      auto proj = m_camera.GetProjectionMat(w, h);

      if (button_lock.IsMouseOverButton(view, proj, glm::vec2(mouseX, mouseY),
                                        w, h)) {
        OnLockClick();
      }
      if (button_key.IsMouseOverButton(view, proj, glm::vec2(mouseX, mouseY), w,
                                       h)) {
        OnKeyClick();
      }
      if (button_rock.IsMouseOverButton(view, proj, glm::vec2(mouseX, mouseY),
                                        w, h)) {
        OnRockClick();
      }
    }
  } else {
    m_first_click = true;
  }

  if (glfwGetKey(&window, GLFW_KEY_E)) {
    m_clickCounter.click();
    soundManager.PlaySound("beep");
  }

  ///// wasd debug controls
  // float sensitivity = 1.0f;
  // if (glfwGetKey(&window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
  //	sensitivity = 1.5f;
  // }
  // float move = 0.01f * sensitivity;

  // if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS) {
  //	// locy += 0.001f;
  //	m_camera.ShiftCamera(0.0f, move, 0.0f);
  // }
  // if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS) {
  //	// locy -= 0.001f;
  //	m_camera.ShiftCamera(0.0f, -move, 0.0f);
  // }
  // if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS) {
  //	// locx -= 0.001f;
  //	m_camera.ShiftCamera(-move, 0.0f, 0.0f);
  // }
  // if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS) {
  //	// locx += 0.001f;
  //	m_camera.ShiftCamera(move, 0.0f, 0.0f);
  // }
  // if (glfwGetKey(&window, GLFW_KEY_SPACE) == GLFW_PRESS) {
  //	m_camera.ShiftCamera(0.0f, 0.0f, -move);
  // }
  // if (glfwGetKey(&window, GLFW_KEY_C) == GLFW_PRESS) {
  //	m_camera.ShiftCamera(0.0f, 0.0f, move);
  // }
}

void GameLoopScene::DrawHallway(glm::mat4 view, glm::mat4 projection,
                                Renderer &renderer) {
  int segs = 10;
  float wall_seg_sizes = 1.0f;
  float wall_height = 1.0f;
  float hallway_width = 1.0f;

  renderer.BeginBatchDraw(segs);
  for (int i = 0; i < segs; i++) {

    SpriteInstance wall_left;
    wall_left.position = glm::vec3(-hallway_width / 2, 0, -(float)i + 0.5f);
    wall_left.size = glm::vec2(1.0f, 1.0f);
    wall_left.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    wall_left.texture = &wall_texture;
    renderer.SubmitSprite(wall_left);
  }
  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f),
                                   glm::vec3(0.0f, 1.0f, 0.0f));
  renderer.RendBatch(rotation, view, projection);

  renderer.BeginBatchDraw(segs);
  for (int i = 0; i < segs; i++) {

    SpriteInstance wall_right;
    wall_right.position = glm::vec3(hallway_width / 2, 0, -(float)i + 0.5f);
    wall_right.size = glm::vec2(1.0f, 1.0f);
    wall_right.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    wall_right.texture = &wall_texture;
    renderer.SubmitSprite(wall_right);
  }
  glm::mat4 rotation2 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
  renderer.RendBatch(rotation2, view, projection);

  renderer.BeginBatchDraw(segs);
  for (int i = 0; i < segs; i++) {

    SpriteInstance sceiling;
    sceiling.position = glm::vec3(0, wall_height * 0.5f, -(float)i + 0.5f);
    sceiling.size = glm::vec2(1.0f, 1.0f);
    sceiling.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sceiling.texture = &ceiling_texture;
    renderer.SubmitSprite(sceiling);
  }
  glm::mat4 rotation3 = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f),
                                    glm::vec3(1.0f, 0.0f, 0.0f));
  renderer.RendBatch(rotation3, view, projection);

  renderer.BeginBatchDraw(segs);
  for (int i = 0; i < segs; i++) {

    SpriteInstance sfloor;
    sfloor.position = glm::vec3(0, -wall_height * 0.5f, -(float)i + 0.5f);
    sfloor.size = glm::vec2(1.0f, 1.0f);
    sfloor.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    sfloor.texture = &floor_texture;
    renderer.SubmitSprite(sfloor);
  }
  renderer.RendBatch(rotation3, view, projection);

  SpriteInstance door;
  door.position = glm::vec3(hallway_width / 2, -0.05, -1.0f);
  door.size = glm::vec2(0.5f, 0.9f);
  door.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  door.texture = &door_texture;

  if (escaped) {
    glm::mat4 doorrot = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
    renderer.BeginBatchDraw(1);
    SpriteInstance door2;
    door2.position = glm::vec3((hallway_width / 2) - 0.25f, -0.05, -1.25f);
    door2.size = glm::vec2(0.5f, 0.9f);
    door2.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    door2.texture = &door_texture;
    renderer.SubmitSprite(door2);
    renderer.RendBatch(doorrot, view, projection);

    door.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
  }

  renderer.BeginBatchDraw(1);
  renderer.SubmitSprite(door);
  renderer.RendBatch(rotation2, view, projection);

  if (!rock_clicked) {
    renderer.BeginBatchDraw(1);
    SpriteInstance rock;
    rock.position = button_rock.m_worldPosition;
    rock.size = button_rock.m_size;
    rock.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    rock.texture = &rock_texture;
    renderer.SubmitSprite(rock);
    renderer.RendBatch(view, projection);
  }

  if (rock_clicked && !key) {
    renderer.BeginBatchDraw(1);
    SpriteInstance key;
    key.position = button_key.m_worldPosition;
    key.size = button_key.m_size;
    key.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    key.texture = &key_texture;
    renderer.SubmitSprite(key);
    renderer.RendBatch(view, projection);
  }
}
