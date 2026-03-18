cmake_minimum_required(VERSION 3.4...4.0)

project(RoundAndRuthless VERSION 1.0
      DESCRIPTION "Round And Ruthless,
      a short horror game about liver and roundies"
      LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -WX-")
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -WX-")
else()
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++23")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
endif()

#External libraries
add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/single_headers)
add_subdirectory(external/glm)

set(LIBTYPE STATIC CACHE STRING "" FORCE)
set(ALSOFT_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(external/openal-soft)

#ImGui
file(GLOB imgui_src
"external/imgui/*.cpp"
"external/imgui/*.h"
"external/imgui/misc/cpp/imgui_stdlib.*"
external/imgui/backends/imgui_impl_opengl3.cpp
external/imgui/backends/imgui_impl_opengl3.h
external/imgui/backends/imgui_impl_glfw.cpp
external/imgui/backends/imgui_impl_glfw.h
)
add_library(imgui STATIC ${imgui_src})
target_include_directories(imgui PUBLIC
  external/imgui
  external/imgui/backends
)
target_link_libraries(imgui PUBLIC
  glfw
)

#my libraries and resources
#garden_engine
set(garden_engine_src_files
  garden_engine/Engine.cpp
  garden_engine/Renderer.cpp
  garden_engine/Shader.cpp
  garden_engine/Texture.cpp
  garden_engine/Camera.cpp
  garden_engine/ClickCounter.cpp
  garden_engine/Audio.cpp
  garden_engine/Approacher.cpp
  garden_engine/Button.cpp

  # include files included for IDE sake
  garden_engine/Engine.hpp
  garden_engine/Renderer.hpp
  garden_engine/Shader.hpp
  garden_engine/Texture.hpp
  garden_engine/Camera.hpp
  garden_engine/ClickCounter.hpp
  garden_engine/gl_debug.hpp
  garden_engine/Audio.hpp
  garden_engine/Approacher.hpp
  garden_engine/Scene.hpp
  garden_engine/Button.hpp
)

add_library(garden_engine STATIC ${garden_engine_src_files})
target_include_directories(garden_engine PUBLIC garden_engine)
target_compile_features(garden_engine PUBLIC cxx_std_23)
target_link_libraries(garden_engine PUBLIC
  glad
  glfw
  single_headers
  glm
  imgui
  OpenAL
)

#Copy Resources
add_custom_target(copy_resources
COMMENT "Copying Resources"
)

add_custom_command(TARGET copy_resources PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
"${CMAKE_CURRENT_SOURCE_DIR}/res/"
"${CMAKE_CURRENT_BINARY_DIR}/res/"
)

#GamePlay code
set(GamePlay_files
game/GameLoopScene.cpp
game/LiverGameState.cpp

game/IntroScene.hpp
game/GameLoopScene.hpp
game/LiverGameState.hpp
game/ExitScene.hpp
)
add_library(GamePlay STATIC ${GamePlay_files})
target_include_directories(GamePlay PUBLIC game)
target_compile_features(GamePlay PUBLIC cxx_std_23)
target_link_libraries(GamePlay PUBLIC garden_engine)


#RoundAndRuthless executable
if(MSVC)
  set(APP_ICON_RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/resources.rc")
endif()

add_executable(${PROJECT_NAME} game/RoundAndRuthless.cpp ${APP_ICON_RESOURCE})
target_link_libraries(${PROJECT_NAME} PUBLIC
garden_engine
GamePlay
)
add_dependencies(${PROJECT_NAME} copy_resources)
