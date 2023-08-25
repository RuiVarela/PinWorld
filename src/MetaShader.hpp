#pragma once

#include "Lang.hpp"

#include "raylib.h"

namespace pw {

    struct MetaShaderContext;

    struct MetaShaderPy;
    struct MetaShaderCpp;
    struct MetaShaderGif;
    struct MetaShaderWater;

    typedef float(*MetaShaderFunction)(MetaShaderContext&, int const, Vector2 const&, float const);

    struct MetaShaderInfo {
        std::string name;
        MetaShaderFunction function;
    };
    using MetaShadersInfo = std::vector<MetaShaderInfo>;

    struct MetaShaderContext {
        MetaShadersInfo shaders;                // available shaders
        MetaShaderInfo shader;                  // active shader

        std::shared_ptr<MetaShaderGif> gif;     // gif context
        std::shared_ptr<MetaShaderPy> py;       // python context 
        std::shared_ptr<MetaShaderWater> wtr;   // native context
        std::shared_ptr<MetaShaderCpp> cpp;     // native context
    };

    std::vector<std::string> platformShadersFiles(std::string const& kind);

    void setupMetaShaders(MetaShaderContext& context);
    void updateContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time);

    void setupPyMetaShaders(MetaShaderContext& context);
    void updatePyContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time);

    void setupGifMetaShaders(MetaShaderContext& context);
    void updateGifContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time);

    void setupWaterMetaShaders(MetaShaderContext& context);
    void updateWaterContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time);
}