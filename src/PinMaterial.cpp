#include "Text.hpp"

#include "PinMaterial.hpp"
#include "raylib.h"
#include "rlgl.h"
#include "config.h"


// https://github.com/raysan5/raylib/blob/0c126af7171e51fff9f94c4f2e498f43f60d617b/src/rlgl.h#L4505

static auto buildCode() {
    std::string vs = 
R"V0G0N(
    #version 330
    
    precision highp float;

    in vec3 vertexPosition;
    in vec2 vertexTexCoord;
    in float vertexId;
    in float vertexPin;

    out vec2 fragTexCoord;
    out vec3 positionInCameraSpace;
    out float pinHeight;

    uniform vec2 canvasSize;
    uniform vec2 canvasStartPosition;
    uniform float canvasPinSize;

    uniform mat4 matView;
    uniform mat4 matProjection;

    void main() {
        fragTexCoord = vertexTexCoord;
        
        //float index = gl_InstanceID;
        float index = vertexId;
        float y = floor(index / canvasSize.x);
        float x = index - y * canvasSize.x;

        pinHeight = vertexPin;
        float height = vertexPin * canvasPinSize * 10.0;

        vec4 position = vec4(
            vertexPosition.x + x * canvasPinSize - canvasStartPosition.x,
            vertexPosition.y + height,
            vertexPosition.z + y * canvasPinSize - canvasStartPosition.y,
            1.0);

        positionInCameraSpace = (matView * position).xyz;
        gl_Position = matProjection * matView * position;
    }
)V0G0N";


std::string ps = R"V0G0N(
    #version 330
    precision highp float;
    
    in vec2 fragTexCoord;
    in vec3 positionInCameraSpace;
    in float pinHeight;

    const float fogNear = 20.0;
    const float fogFar = 60.0;

    out vec4 finalColor;

    void main() {
        float cameraDistance = length(positionInCameraSpace.xyz);
        float fogAmount = 1.0 - smoothstep(fogNear, fogFar, cameraDistance);

        //float border_size = min(fwidth(fragTexCoord.x), fwidth(fragTexCoord.y)) * 4.0;
        float border_size = 0.05;
        
        vec2 tex_position = fragTexCoord.xy - vec2(0.5); // center on the face center make the range -0.5 <-> 0.5
        tex_position = abs(tex_position); // fold space, make the range 0.5 <-> 0.5
        tex_position = smoothstep(border_size, 0.0, vec2(0.5) - tex_position);
        float border_mask = max(tex_position.x, tex_position.y);

        vec4 mainColor = mix(vec4(0.574, 0.13, 0.22, 1.0), vec4(0.90, 0.16, 0.22, 1.0), pinHeight);
        vec4 borderColor = vec4(1.0, 0.63, 0.0, 1.0);

        finalColor = mix(mainColor, borderColor, border_mask * fogAmount);
    }
)V0G0N";

#if defined(GRAPHICS_API_OPENGL_ES2)
    pw::replace(vs, "#version 330", "#version 100");
    pw::replace(vs, " in float ", " attribute float ");
    pw::replace(vs, " in vec2 ", " attribute vec2 ");
    pw::replace(vs, " in vec3 ", " attribute vec3 ");
    pw::replace(vs, " out float ", " varying float ");
    pw::replace(vs, " out vec2 ", " varying vec2 ");
    pw::replace(vs, " out vec3 ", " varying vec3 ");
    pw::replace(vs, " out vec4 ", " varying vec4 ");

    pw::replace(ps, "#version 330", "#version 100");
    pw::replace(ps, " in float ", " varying float ");
    pw::replace(ps, " in vec2 ", " varying vec2 ");
    pw::replace(ps, " in vec3 ", " varying vec3 ");
    pw::replace(ps, " in vec4 ", " varying vec3 ");
    pw::replace(ps, " out vec4 finalColor;", "");
    pw::replace(ps, " finalColor ", " gl_FragColor ");
#endif

    return std::make_pair(vs, ps);
}


namespace pw {

    Material loadPinMaterial(
        Mesh& mesh, std::vector<float>& pins,
        int w, int h, 
        float x_start, float y_start, 
        float pin_size
    ) {
        int count = int(pins.size());

        Material material;
        material.maps = (MaterialMap*)calloc(MAX_MATERIAL_MAPS, sizeof(MaterialMap));

        const auto [vs, ps] = buildCode();
        material.shader = LoadShaderFromMemory(vs.c_str(), ps.c_str());

        int loc_vertex_id = rlGetLocationAttrib(material.shader.id, "vertexId");
        int loc_vertex_pin = rlGetLocationAttrib(material.shader.id, "vertexPin");

        int loc_canvas_size = rlGetLocationUniform(material.shader.id, "canvasSize");
        int loc_canvas_start_position = rlGetLocationUniform(material.shader.id, "canvasStartPosition");
        int loc_canvas_pin_size = rlGetLocationUniform(material.shader.id, "canvasPinSize");


        // setup the uniforms
        rlEnableShader(material.shader.id);
            float canvas_size[] = { float(w), float(h) };
            rlSetUniform(loc_canvas_size, canvas_size, RL_SHADER_UNIFORM_VEC2, 1);

            float start_position[] = { x_start, y_start };
            rlSetUniform(loc_canvas_start_position, start_position, RL_SHADER_UNIFORM_VEC2, 1);

            rlSetUniform(loc_canvas_pin_size, &pin_size, RL_SHADER_UNIFORM_FLOAT, 1);
        rlDisableShader();

        material.maps[MATERIAL_MAP_DIFFUSE].color = RED; 

        //
        // setup ids
        //
        {
            std::vector<float> ids(count, 0.0f);

            for (size_t i = 0; i != ids.size(); ++i)
                ids[i] = float(i);

            rlEnableVertexArray(mesh.vaoId);
                mesh.vboId[VBO_IDS] = rlLoadVertexBuffer(ids.data(), count * 1 * sizeof(float), false);
                rlSetVertexAttribute(loc_vertex_id, 1, RL_FLOAT, 0, 0, 0);
                rlEnableVertexAttribute(loc_vertex_id);
                rlSetVertexAttributeDivisor(loc_vertex_id, 1);
            rlDisableVertexArray();
        }


        //
        // setup pins
        //
        {
            rlEnableVertexArray(mesh.vaoId);
                mesh.vboId[VBO_PIN] = rlLoadVertexBuffer(pins.data(), count * 1 * sizeof(float), true);
                rlSetVertexAttribute(loc_vertex_pin, 1, RL_FLOAT, 0, 0, 0);
                rlEnableVertexAttribute(loc_vertex_pin);
                rlSetVertexAttributeDivisor(loc_vertex_pin, 1);
            rlDisableVertexArray();
        }

        return material;
    }
}