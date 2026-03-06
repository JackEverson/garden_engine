
#include "Config.hpp"
#include "MainScene.hpp"
#include <glm/gtc/random.hpp>

#include "GLFW/glfw3.h"

void MainScene::updatePlayerPhysics(Player &player, float deltaTime) {

  if (!player.isAlive) {
    player.size *= 0.99f;
    m_player_sprite.color -= glm::vec4(0.01f, 0.01f, 0.0f, 0.01f);
    return;
  }

  checkWallCollision(m_player, 1920 / 1080);

  resolvePlatformCollisions(m_player);

  if (!player.isGrounded) {
    player.velocity.y += m_gravity * deltaTime;
  } else {
    player.velocity.y = 0.0f;
  }

  player.position += player.velocity * deltaTime;

  if (player.position.y < m_fail_y && player.isAlive) {
    player.isAlive = false;

    if (player.position.y > m_config.highscore) {
      m_config.highscore = player.position.y;
      SaveConfig(m_config);
    }
  }
}

void MainScene::checkWallCollision(Player &player, float tubeX) {

  if (player.position.x >= tubeX) {
    player.position.x = tubeX;
    player.velocity.x *= -1.0f;
  } else if (player.position.x <= -tubeX) {
    player.position.x = -tubeX;
    player.velocity.x *= -1.0f;
  }
}

void MainScene::generatePlatforms(float &playerY) {

  const float m_platformsAbovePlayer = 20.0f;

  while (m_lastplatform_y < playerY + m_platformsAbovePlayer) {

    float spacing = glm::linearRand(m_minPlatSpacingY, m_maxPlatSpacingY);
    m_lastplatform_y += spacing;
    float x = glm::linearRand(m_minPlatSpacingX, m_maxPlatSpacingX);
    m_platforms.push_back({glm::vec2(x, m_lastplatform_y),
                           glm::vec2(m_platform_width, m_platform_height)});
  }

  m_platforms.erase(std::remove_if(m_platforms.begin(), m_platforms.end(),
                                   [playerY](const Platform &platform) {
                                     return platform.position.y <
                                            playerY - 20.0f;
                                   }),
                    m_platforms.end());

  // need to include walls and background here too
}

void MainScene::resolvePlatformCollisions(Player &player) {

  float player_top = player.position.y + player.size.y * 0.5f;
  float player_bottom = player.position.y - player.size.y * 0.5f;
  float player_left = player.position.x - player.size.x * 0.5f;
  float player_right = player.position.x + player.size.x * 0.5f;

  if (player.isGrounded) {
    if (!checkPlatformCollisions(player_top, player_bottom, player_left,
                                 player_right, *player.currentPlatform)) {
      player.isGrounded = false;
      player.currentPlatform = nullptr;
      // std::println("Left platform at y: {}", platform.position.y);
    }
    return;
  }

  for (auto &platform : m_platforms) {
    if (checkPlatformCollisions(player_top, player_bottom, player_left,
                                player_right, platform) &&
        player.velocity.y < 0.0f) {
      player.position.y =
          platform.position.y + platform.size.y * 0.50f + player.size.y * 0.5f;
      player.isGrounded = true;
      player.currentPlatform = &platform;
      // std::println("Landed on platform at y: {}", platform.position.y);
      return;
    }
  }

  player.isGrounded = false;
}

bool MainScene::checkPlatformCollisions(const float &playerTop,
                                        const float &playerBottom,
                                        const float &playerLeft,
                                        const float &playerRight,
                                        const Platform &platform) {

  float platform_top = platform.position.y + platform.size.y * 0.5f;
  // float platform_bottom = platform.position.y + platform.size.y * -0.5f;
  float platform_left = platform.position.x - platform.size.x * 0.5f;
  float platform_right = platform.position.x + platform.size.x * 0.5f;

  return playerLeft < platform_right && playerRight > platform_left &&
         playerBottom <= platform_top + platform.size.y * 0.1f &&
         playerBottom >= platform_top - platform.size.y * 1.0f;
}

void MainScene::handlePlayerInput(Player &player, GLFWwindow &window,
                                  float delta) {

  bool squish = false;

  float adjust = 1.0f;
  if (glfwGetKey(&window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
    adjust = 2.0f;
  }

  if (glfwGetKey(&window, GLFW_KEY_W) == GLFW_PRESS ||
      glfwGetKey(&window, GLFW_KEY_SPACE) == GLFW_PRESS ||
      glfwGetKey(&window, GLFW_KEY_UP) == GLFW_PRESS) {
    if (m_player.isGrounded) {
      m_player.position.y += 0.001f;
      m_player.velocity.y += m_player.jump_power * adjust;
      m_player.isGrounded = false;
    }
  }
  if (glfwGetKey(&window, GLFW_KEY_S) == GLFW_PRESS ||
      glfwGetKey(&window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    if (m_player.isGrounded) {
      m_player.velocity.x *= 5 * delta;
    }
    squish = true;
  }
  if (glfwGetKey(&window, GLFW_KEY_A) == GLFW_PRESS ||
      glfwGetKey(&window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    m_player.velocity.x -= m_player.run_speed * delta * adjust;
  }
  if (glfwGetKey(&window, GLFW_KEY_D) == GLFW_PRESS ||
      glfwGetKey(&window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    m_player.velocity.x += m_player.run_speed * delta * adjust;
  }

  if (squish) {
    m_player.size.y = 0.05f;
  } else {
    m_player.size.y = 0.06f;
  }

  if (glfwGetKey(&window, GLFW_KEY_R) == GLFW_PRESS) {
    m_restart = true;
  }

  bool left_click = false;
  if (!m_clicked &&
      glfwGetMouseButton(&window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
    left_click = true;
  }
}
