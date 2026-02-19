#pragma once

#include <fstream>

inline void SaveHighScores(float &Score) {

  std::ofstream file("./highscore.txt");

  if (!file.is_open()) {
    return; // failed to open file
  }

  file << Score;

  file.close();
}

inline float LoadHighScore() {
  std::ifstream file("./highscore.txt");

  if (!file.is_open()) {
    return 0.0f;
  }

  float s;
  file >> s;

  file.close();
  return s;
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