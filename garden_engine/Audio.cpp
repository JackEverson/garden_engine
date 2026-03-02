#include "Audio.hpp"
#include <iostream>
#include <vector>

// Include the single-header libraries
#define STB_VORBIS_IMPLEMENTATION
#include "stb_vorbis.c"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

bool SimpleSoundManager::Initialize() {

  _lock.lock();

  if (!_is_initalized) {
    std::cout << "Initializing Simple Sound Manager..." << std::endl;

    // Step 1: Open the default audio device
    device = alcOpenDevice(nullptr);
    if (!device) {
      std::cerr << "Failed to open OpenAL device" << std::endl;
      return false;
    }

    // Step 2: Create an audio context
    context = alcCreateContext(device, nullptr);
    if (!context) {
      std::cerr << "Failed to create OpenAL context" << std::endl;
      alcCloseDevice(device);
      device = nullptr;
      return false;
    }

    // Step 3: Make the context current (like OpenGL)
    if (!alcMakeContextCurrent(context)) {
      std::cerr << "Failed to make OpenAL context current" << std::endl;
      alcDestroyContext(context);
      alcCloseDevice(device);
      context = nullptr;
      device = nullptr;
      return false;
    }

    std::cout << "OpenAL initialized successfully!" << std::endl;

    // Print some info about our audio setup
    std::cout << "OpenAL Vendor: " << alGetString(AL_VENDOR) << std::endl;
    std::cout << "OpenAL Renderer: " << alGetString(AL_RENDERER) << std::endl;
    std::cout << "OpenAL Version: " << alGetString(AL_VERSION) << std::endl;
    _is_initalized = true;
  }

  _lock.unlock();
  return true;
}

bool SimpleSoundManager::IsInitalized() { return _is_initalized; }

void SimpleSoundManager::Shutdown() {
  std::cout << "Shutting down Simple Sound Manager..." << std::endl;

  // Clean up all sounds
  for (auto &pair : sounds) {
    Sound &sound = pair.second;
    if (sound.source != 0) {
      alDeleteSources(1, &sound.source);
    }
    if (sound.buffer != 0) {
      alDeleteBuffers(1, &sound.buffer);
    }
  }
  sounds.clear();

  // Clean up OpenAL
  if (context) {
    alcMakeContextCurrent(nullptr);
    alcDestroyContext(context);
    context = nullptr;
  }

  if (device) {
    alcCloseDevice(device);
    device = nullptr;
  }

  std::cout << "Sound Manager shut down" << std::endl;
}

bool SimpleSoundManager::LoadSound(const std::string &name,
                                   const std::string &filepath) {
  std::cout << "Loading sound: " << name << " from " << filepath << std::endl;

  // Check if already loaded
  if (sounds.find(name) != sounds.end()) {
    std::cout << "Sound " << name << " already loaded" << std::endl;
    return true;
  }

  Sound sound;

  // Create OpenAL buffer and source
  alGenBuffers(1, &sound.buffer);
  alGenSources(1, &sound.source);

  // Check for errors
  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "Failed to create OpenAL buffer/source for " << name
              << std::endl;
    return false;
  }

  // Try to load the file (check extension)
  bool loaded = false;
  std::string ext = filepath.substr(filepath.find_last_of('.'));

  if (ext == ".wav" || ext == ".WAV") {
    loaded = LoadWAV(filepath, sound.buffer);
  } else if (ext == ".ogg" || ext == ".OGG") {
    loaded = LoadOGG(filepath, sound.buffer);
  } else {
    std::cerr << "Unsupported file format: " << ext << std::endl;
  }

  if (!loaded) {
    // Clean up on failure
    alDeleteSources(1, &sound.source);
    alDeleteBuffers(1, &sound.buffer);
    return false;
  }

  // Connect the buffer to the source
  alSourcei(sound.source, AL_BUFFER, sound.buffer);

  // Set some basic properties
  alSourcef(sound.source, AL_PITCH, 1.0f);
  alSourcef(sound.source, AL_GAIN, masterVolume);

  // Store the sound
  sounds[name] = sound;

  std::cout << "Successfully loaded sound: " << name << std::endl;
  return true;
}

void SimpleSoundManager::PlaySound(const std::string &name) {
  auto it = sounds.find(name);
  if (it == sounds.end()) {
    std::cerr << "Sound not found: " << name << std::endl;
    return;
  }

  // Simple play - just start from the beginning each time
  ALuint source = it->second.source;
  alSourcePlay(source);

  // Check for errors
  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "Failed to play sound: " << name << std::endl;
  }
}

void SimpleSoundManager::SetMasterVolume(float volume) {
  // Clamp volume between 0 and 1
  masterVolume = std::max(0.0f, std::min(1.0f, volume));

  // Update all existing sources
  for (auto &pair : sounds) {
    alSourcef(pair.second.source, AL_GAIN, masterVolume);
  }
}

bool SimpleSoundManager::LoadWAV(const std::string &filepath, ALuint buffer) {
  // Use dr_wav to load the file
  drwav wav;
  if (!drwav_init_file(&wav, filepath.c_str(), nullptr)) {
    std::cerr << "Failed to load WAV file: " << filepath << std::endl;
    return false;
  }

  // Read all samples
  size_t sampleCount = wav.totalPCMFrameCount * wav.channels;
  std::vector<int16_t> samples(sampleCount);

  drwav_read_pcm_frames_s16(&wav, wav.totalPCMFrameCount, samples.data());

  // Determine OpenAL format
  ALenum format;
  if (wav.channels == 1) {
    format = AL_FORMAT_MONO16;
  } else if (wav.channels == 2) {
    format = AL_FORMAT_STEREO16;
  } else {
    std::cerr << "Unsupported channel count in WAV: " << wav.channels
              << std::endl;
    drwav_uninit(&wav);
    return false;
  }

  // Upload to OpenAL
  alBufferData(buffer, format, samples.data(), samples.size() * sizeof(int16_t),
               wav.sampleRate);

  drwav_uninit(&wav);

  // Check for OpenAL errors
  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "OpenAL error loading WAV: " << filepath << std::endl;
    return false;
  }

  std::cout << "WAV loaded: " << wav.channels << " channels, " << wav.sampleRate
            << " Hz, " << wav.totalPCMFrameCount << " frames" << std::endl;

  return true;
}

bool SimpleSoundManager::LoadOGG(const std::string &filepath, ALuint buffer) {
  int error;
  stb_vorbis *vorbis =
      stb_vorbis_open_filename(filepath.c_str(), &error, nullptr);

  if (!vorbis) {
    std::cerr << "Failed to load OGG file: " << filepath << " (error: " << error
              << ")" << std::endl;
    return false;
  }

  // Get file info
  stb_vorbis_info info = stb_vorbis_get_info(vorbis);

  // Read all samples
  int sampleCount = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;
  std::vector<int16_t> samples(sampleCount);

  stb_vorbis_get_samples_short_interleaved(vorbis, info.channels,
                                           samples.data(), sampleCount);

  // Determine OpenAL format
  ALenum format;
  if (info.channels == 1) {
    format = AL_FORMAT_MONO16;
  } else if (info.channels == 2) {
    format = AL_FORMAT_STEREO16;
  } else {
    std::cerr << "Unsupported channel count in OGG: " << info.channels
              << std::endl;
    stb_vorbis_close(vorbis);
    return false;
  }

  // Upload to OpenAL
  alBufferData(buffer, format, samples.data(), samples.size() * sizeof(int16_t),
               info.sample_rate);

  stb_vorbis_close(vorbis);

  // Check for OpenAL errors
  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "OpenAL error loading OGG: " << filepath << std::endl;
    return false;
  }

  std::cout << "OGG loaded: " << info.channels << " channels, "
            << info.sample_rate << " Hz" << std::endl;

  return true;
}

// NEW: Play sound with loop option
void SimpleSoundManager::PlaySound(const std::string &name, bool loop) {
  auto it = sounds.find(name);
  if (it == sounds.end()) {
    std::cerr << "Sound not found: " << name << std::endl;
    return;
  }

  Sound &sound = it->second;

  // Set looping
  alSourcei(sound.source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  sound.isLooping = loop;

  // Update volume before playing
  UpdateSoundVolume(name);

  // Play the sound
  alSourcePlay(sound.source);

  if (alGetError() != AL_NO_ERROR) {
    std::cerr << "Failed to play sound: " << name << std::endl;
  } else {
    std::cout << "Playing " << (loop ? "looping " : "") << "sound: " << name
              << std::endl;
  }
}

// NEW: Stop a specific sound
void SimpleSoundManager::StopSound(const std::string &name) {
  auto it = sounds.find(name);
  if (it == sounds.end()) {
    std::cerr << "Sound not found: " << name << std::endl;
    return;
  }

  alSourceStop(it->second.source);
  std::cout << "Stopped sound: " << name << std::endl;
}

// NEW: Check if a sound is playing
bool SimpleSoundManager::IsSoundPlaying(const std::string &name) {
  auto it = sounds.find(name);
  if (it == sounds.end()) {
    return false;
  }

  ALint state;
  alGetSourcei(it->second.source, AL_SOURCE_STATE, &state);
  return state == AL_PLAYING;
}

// NEW: Set volume for a specific sound
void SimpleSoundManager::SetSoundVolume(const std::string &name, float volume) {
  auto it = sounds.find(name);
  if (it == sounds.end()) {
    std::cerr << "Sound not found: " << name << std::endl;
    return;
  }

  // Store the volume and update
  it->second.volume = std::max(0.0f, std::min(1.0f, volume));
  UpdateSoundVolume(name);
}

// Helper function to update sound volume (considers master volume and music
// volume)
void SimpleSoundManager::UpdateSoundVolume(const std::string &name) {
  auto it = sounds.find(name);
  if (it == sounds.end())
    return;

  Sound &sound = it->second;

  // Calculate final volume
  float finalVolume = sound.volume * masterVolume;

  // If this is background music, apply music volume too
  if (name == currentBackgroundMusic) {
    finalVolume *= musicVolume;
  }

  alSourcef(sound.source, AL_GAIN, finalVolume);
}

// NEW: Easy background music control
void SimpleSoundManager::PlayBackgroundMusic(const std::string &name,
                                             float volume) {
  // Stop current background music if any
  if (!currentBackgroundMusic.empty() &&
      IsSoundPlaying(currentBackgroundMusic)) {
    StopSound(currentBackgroundMusic);
  }

  // Set as current background music
  currentBackgroundMusic = name;

  // Set volume and play looping
  SetSoundVolume(name, volume);
  PlaySound(name, true); // true = loop

  std::cout << "Started background music: " << name << std::endl;
}

void SimpleSoundManager::StopBackgroundMusic() {
  if (!currentBackgroundMusic.empty()) {
    StopSound(currentBackgroundMusic);
    std::cout << "Stopped background music: " << currentBackgroundMusic
              << std::endl;
    currentBackgroundMusic = "";
  }
}

void SimpleSoundManager::SetMusicVolume(float volume) {
  musicVolume = std::max(0.0f, std::min(1.0f, volume));

  // Update current background music volume
  if (!currentBackgroundMusic.empty()) {
    UpdateSoundVolume(currentBackgroundMusic);
  }

  std::cout << "Music volume set to: " << musicVolume << std::endl;
}
