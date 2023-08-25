# PinWorld
PinWorld is 3d pin canvas. You can use it to create metashaders that control each pin heigh. 
It is a simple 3d engine that uses raylib and raygui.

## Features
- Fast pins renderer
- Minimal dependencies
- Simple Metashader to position pins
- Cpp Metashader - you can develop your own metashaders in C++
- Py Metashader - you can develop your own metashaders in Python, support via [pocketpy](https://pocketpy.dev/)
- Py animation code is not fast, but it can be usefull for prototyping
- Gif Metashader - it plays out the gif animations
- Water Metashader loosely based on [Acerola's video](https://www.youtube.com/watch?v=PH9q0HNBjT4&list=PLFTSYFO3lrKw35oVgO_GzXbvu7medjsG6&index=4)
- Works on Mac, Windows and [WASM - web demo](https://pinworld.demanda.pt/)

## Controls
- menu_key = KEY_M
- up_key = KEY_W
- down_key = KEY_S
- left_key = KEY_A
- right_key = KEY_D
- forward_key = KEY_Q
- backward_key = KEY_E
- next_shader_key = KEY_RIGHT
- next_shader_fast_key = KEY_DOWN
- previous_shader_key = KEY_LEFT
- previous_shader_fast_key = KEY_UP

## Development
```bash
git git@github.com:RuiVarela/PinWorld.git
cd PinWorld

mkdir build
cd build

# General
cmake ..
cmake --build .

# To build a Release version on Linux and Mac:
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
cmake --build . -- -j 8

# Build a release version for mac with arm support
cmake -DCMAKE_OSX_ARCHITECTURES="arm64x86_64" -DCMAKE_BUILD_TYPE=Release .. 
cmake --build . -- -j 8
lipo -archs Senos.app/Contents/MacOS/Senos

# To build a Release version on Windows with the VisualStudio toolchain:
cmake ..
cmake --build . --config Release
cmake --build . --config MinSizeRel

# Emscripten
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/Users/ruivarela/projects/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DPLATFORM=Web ..
cmake --build . -- -j 8
# run web version using a tiny webserver

python3 web/wasm-server.py
```

# Credits
- [raylib](https://www.raylib.com/)
- [raygui](https://github.com/raysan5/raygui)
- [nlohmann json](https://github.com/nlohmann/json)
- [pocketpy](https://pocketpy.dev/)
