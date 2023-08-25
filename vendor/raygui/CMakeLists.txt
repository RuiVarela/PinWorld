set(RAYGUI_VERSION 3.6)

message(STATUS "Using raygui ${RAYGUI_VERSION} ...")

FetchContent_Declare(
    raygui
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.zip
)

FetchContent_MakeAvailable(raygui)

set(RAYGUI_DIR ${raygui_SOURCE_DIR}/src CACHE STRING "raygui source dir")