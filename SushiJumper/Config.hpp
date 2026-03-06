#pragma once

#include <fstream>

static std::string option_file = "./sushiconfig.txt";
static std::string highscore_label = "highscore";
static std::string volume_label = "volume";
static std::string fullscreen_label = "fullscreen";

struct SushiJumperConfig {
  float highscore = 0.0f;
  float volume = 1.0f;
  bool fullscreen = true;
};

inline void SaveConfig(SushiJumperConfig &config) {

  std::ofstream file(option_file);

  if (!file.is_open()) {
    return; // failed to open file
  }

  file << highscore_label << " " << config.highscore << std::endl;
  file << volume_label << " " << config.volume << std::endl;
  file << fullscreen_label << " " << config.fullscreen << std::endl;

  file.close();
}

inline SushiJumperConfig LoadConfig() {
  std::ifstream file(option_file);

  SushiJumperConfig config;
  std::string label;

  if (file.is_open()) {
    while (file >> label) {
      if (label == highscore_label) {
        file >> config.highscore;
      } else if (label == volume_label) {
        file >> config.volume;
      } else if (label == fullscreen_label) {
        file >> config.fullscreen;
      }
    }
  }
  file.close();
  return config;
}

// #ifdef _WIN32
// #include <windows.h>
// #else
// #include <unistd.h>
// #endif

// #include <filesystem>
// #include <string>

// std::filesystem::path get_binary_directory() {
// #ifdef _WIN32
//     char buffer[MAX_PATH];
//     GetModuleFileNameA(NULL, buffer, MAX_PATH);
//     return std::filesystem::path(buffer).parent_path();
// #else
//     char buffer[1024];
//     ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
//     if (len != -1) {
//         buffer[len] = '\0';
//         return std::filesystem::path(buffer).parent_path();
//     }
//     return "";
// #endif
// }
