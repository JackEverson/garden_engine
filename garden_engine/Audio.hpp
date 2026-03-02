#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <mutex>
#include <string>
#include <unordered_map>

// Step 1: Simple sound loading and playing
class SimpleSoundManager {
public:
  // Singleton pattern - only one sound manager
  static SimpleSoundManager &Instance() {
    static SimpleSoundManager instance;
    return instance;
  }

  // Core functions - keep it minimal
  bool Initialize();
  bool IsInitalized();
  void Shutdown();

  bool LoadSound(const std::string &name, const std::string &filepath);
  void PlaySound(const std::string &name);
  void SetMasterVolume(float volume);

  void PlaySound(const std::string &name, bool loop);
  void StopSound(const std::string &name);
  bool IsSoundPlaying(const std::string &name);
  void SetSoundVolume(const std::string &name, float volume);

  void PlayBackgroundMusic(const std::string &name, float volume = 0.7f);
  void StopBackgroundMusic();
  void SetMusicVolume(float volume);

private:
  // Private constructor for singleton
  SimpleSoundManager() = default;
  ~SimpleSoundManager() = default;

  bool _is_initalized = false;
  std::mutex _lock;

  // OpenAL context
  ALCdevice *device = nullptr;
  ALCcontext *context = nullptr;

  struct Sound {
    ALuint buffer = 0;
    ALuint source = 0;
    bool isLooping = false;
    float volume = 1.0f;
  };

  std::unordered_map<std::string, Sound> sounds;
  float masterVolume = 1.0f;
  float musicVolume = 0.7f; // Separate volume for music

  std::string currentBackgroundMusic = ""; // Track what's playing

  bool LoadWAV(const std::string &filepath, ALuint buffer);
  bool LoadOGG(const std::string &filepath, ALuint buffer);
  void UpdateSoundVolume(const std::string &name);
};
