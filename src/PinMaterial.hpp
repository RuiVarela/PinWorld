#pragma once

#include "raylib.h"
#include "Lang.hpp"

namespace pw {

    constexpr int VBO_IDS = 3;
    constexpr int VBO_PIN = 3;

    Material loadPinMaterial(
        Mesh& mesh, std::vector<float>& pins,
        int w, int h, 
        float x_start, float y_start, 
        float pin_size
    );


 
}