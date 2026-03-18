# garden_engine

A small graphics engine backend for use in making simple graphical applications and games. Currently based around OpenGL (glad), GLFW, GLM, ImGUI, OpenAL-soft, and nothings/stb. 

### [SushiJumper](https://github.com/JackEverson/SushiJumper)
<img width="2544" height="1396" alt="SushiJumper_ScreenShot" src="https://github.com/user-attachments/assets/35cad4ed-041d-4c74-a2dd-b3353f961d9f" />


### RoundAndRuthless (upcoming)
<img width="960" height="540" alt="SushiTunnel" src="https://github.com/user-attachments/assets/7be18234-9738-4a56-906b-69785d93119e" />


Feel free to build and test the program, I always appreciate feedback on how to improve. All external repos are included as submodules.

Steps for building:

```bash
git clone --recurse-submodules https://github.com/JackEverson/RoundAndRuthless.git
cd RoundAndRuthless
cmake -B ./build -S .

# g++
cmake --build ./build
# MSVC
cmake --build ./build --config Release
```




# Attributions 

- gravel_floor.png, modified from [Monstera Production - pexels](https://www.pexels.com/photo/rough-gray-surface-with-stones-7794426/)
- plaster_ceiling.png, modified from [Sasha Martynov - pexels](https://www.pexels.com/photo/white-wall-paint-1260727/)
- concrete_wall.png, modified from [pexels](https://www.pexels.com)

- ghost-scare-vintage.wav, modified from [onderwish - freesound](https://freesound.org/people/onderwish/sounds/457514/)
- ambient-noise.ogg, modified from [patchytherat - freesound](https://freesound.org/people/patchytherat/sounds/535048/)
- beep.wav, modified from [Breviceps - freesound](https://freesound.org/people/Breviceps/sounds/444492/)
