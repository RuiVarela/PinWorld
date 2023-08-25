#include "PinWorld.hpp"
#include "PinMaterial.hpp"

#include "rlgl.h"
#include "raymath.h"
#include "rcamera.h"



static void DrawFPSWithText(int posX, int posY, std::string const& text) {
    Color color = LIME;                         // Good FPS
    int fps = GetFPS();

    if ((fps < 30) && (fps >= 15)) color = ORANGE;  // Warning FPS
    else if (fps < 15) color = RED;             // Low FPS

    DrawText(TextFormat("%2i fps. %s", GetFPS(), text.c_str()), posX, posY, 20, color);
}

namespace pw {

    PinWorld::PinWorld()
        :m_window_width(0.0f), m_window_height(0.0f), m_start_time(0.0)
    {
        m_camera = { 0 };
        m_camera.position = { 10.0f, 5.0f, 5.0f };  // Camera position
        m_camera.target = { 0.0f, 0.0f, 0.0f };      // Camera looking at point
        m_camera.up = { 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
        m_camera.fovy = 45.0f;                       // Camera field-of-view Y
        m_camera.projection = CAMERA_PERSPECTIVE;    // Camera mode type

        memset(&m_pin_mesh, 0, sizeof(m_pin_mesh));
        memset(&m_pin_material, 0, sizeof(m_pin_material));

        m_canvas_divisor = 1;

        float margin_factor = 0.0f;//0.05f;
        m_pin_size_with_margins = 1.0f;
        m_pin_height = 2.0f;
        m_pin_size = m_pin_size_with_margins * (1.0f - 2.0f * margin_factor);
    }

    PinWorld::~PinWorld() {

    }

	void PinWorld::setup() {

		unsigned int flags = FLAG_VSYNC_HINT /*| FLAG_MSAA_4X_HINT */| FLAG_WINDOW_RESIZABLE;

#ifdef PLATFORM_WEB
		SetConfigFlags(flags);

		InitWindow(800, 600, "PinWorld");
#else
		flags |= FLAG_WINDOW_HIDDEN;
		SetConfigFlags(flags);

		InitWindow(1024, 768, "PinWorld");

		int monitor = GetCurrentMonitor();
		int monitor_height = GetMonitorHeight(GetCurrentMonitor());
		int monitor_width = GetMonitorWidth(GetCurrentMonitor());
		int h = int(monitor_height * 0.8f);
		int w = int(h * (16.0f / 9.0f));
		SetWindowSize(w, h);
		SetWindowPosition((monitor_width - w) / 2, (monitor_height - h) / 2);

		ClearWindowState(FLAG_WINDOW_HIDDEN);

		SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
#endif

		//DisableCursor();                // Limit cursor to relative movement inside the window

		computeSizes();

		SetTraceLogLevel(LOG_DEBUG);

        setupMetaShaders(m_meta_shader_context);

        m_menu.setup(m_meta_shader_context);
	}

    void PinWorld::shutdown() {
        m_menu.shutdown();

        UnloadMesh(m_pin_mesh);
        UnloadMaterial(m_pin_material);

        CloseWindow();        // Close window and OpenGL context
    }
    
    void PinWorld::run() {
        while (!WindowShouldClose()) // Detect window close button or ESC key
            step();
    }

    void PinWorld::step() {
        update();
        render();
    }
    
    void PinWorld::render() {
        BeginDrawing();
            renderBackground();

            BeginMode3D(m_camera);
                renderPins();
                //DrawGrid(std::max(m_canvas_width, m_canvas_height) + 10, 1.0f);
            EndMode3D();

            if (m_menu.showing())
                DrawFPSWithText(5, 5, "'m' for cookies");


            m_menu.render();
        EndDrawing();
    }

    // generates a pallete color [side, middle, side] on keypoints [0.0, 0.5, 1.0]
    static Color colorPallete(float factor, Color const& side, Color const& middle) {
        const Vector4 side_n = ColorNormalize(side);
        const Vector4 middle_n = ColorNormalize(middle);

        // position 0.0 -> 1.0
        float position = std::abs(std::fmod(factor, 1.0f));

        float f = position;

        if (position < 0.5f) {
            f = position / 0.5;
        } else { // >= 0.5f
            f = 1.0f - (position - 0.5) / 0.5;
        }
       
        float r = Lerp(side_n.x, middle_n.x, f);
        float g = Lerp(side_n.y, middle_n.y, f);
        float b = Lerp(side_n.z, middle_n.z, f);
        float a = Lerp(side_n.w, middle_n.w, f);

        Color color;
        color.r = (unsigned char)(r * 255.0f);
        color.g = (unsigned char)(g * 255.0f);
        color.b = (unsigned char)(b * 255.0f);
        color.a = (unsigned char)(a * 255.0f);

        return color;
    }

    void PinWorld::renderBackground() {
        ClearBackground(RAYWHITE);

        constexpr const Vector3 up = { 0.0f, 1.0f, 0.0f };
        constexpr const Vector3 right = { 1.0f, 0.0f, 0.0f };

        const float pitch = Vector3Angle(up, GetCameraForward(&m_camera));
        const float yaw = Vector3Angle(right, GetCameraRight(&m_camera));

        {
            //
            // accent a color based on the pitch
            //
            const float yaw_factor = (yaw / PI);
            constexpr const float color_spacing = 0.5f;
            const Color a = SKYBLUE; //SKYBLUE, 
            const Color b = RAYWHITE; //RAYWHITE
            const Color start_color = colorPallete(yaw_factor, a, b);
            const Color end_color = colorPallete(yaw_factor + color_spacing, a, b);
            DrawRectangleGradientH(0, 0, GetRenderWidth(), GetRenderHeight(), start_color, end_color);
        }

        {
            //
            // accent a color based on the pitch
            //
            constexpr const float dead_zone = 0.15f;
            constexpr const float live_zone = 1.0f - dead_zone;
            constexpr const float max_alpha = 0.75f;

            float pitch_factor = ((pitch / PI) - 0.5f) * 2.0f;
            // TraceLog(LOG_DEBUG, "yaw=%.2f pitch=%.2f || pitch_factor=%.2f", yaw, pitch, pitch_factor);

            if (pitch_factor > dead_zone) {

                float alpha = (pitch_factor - dead_zone) / live_zone;
                DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Fade(BROWN, alpha * max_alpha));

            }  else if (pitch_factor < -dead_zone) {

                float alpha = (-pitch_factor - dead_zone) / live_zone;
                DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Fade(BLUE, alpha * max_alpha));

            }
        }
   
    }

    void PinWorld::renderPins() {
        Material& material = m_pin_material;
        Mesh& mesh = m_pin_mesh;

        rlUpdateVertexBuffer(mesh.vboId[VBO_PIN], m_pins.data(), int(m_pins.size() * sizeof(float)), 0);

        rlEnableShader(material.shader.id);

        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_VIEW], rlGetMatrixModelview());
        rlSetUniformMatrix(material.shader.locs[SHADER_LOC_MATRIX_PROJECTION], rlGetMatrixProjection());

        // Try binding vertex array objects (VAO) 
        rlEnableVertexArray(mesh.vaoId);

        int instances = m_canvas_width * m_canvas_height;
        rlDrawVertexArrayElementsInstanced(0, mesh.triangleCount * 3, 0, instances);

        rlDisableVertexArray();

        rlDisableShader();
    }

    void PinWorld::update() {
        //
        // dispatch actions
        //
        MenuActions actions = m_menu.takeActions();
        for (auto& action : actions) {
            if (action.kind == MenuActionKind::ShaderChange) {
                m_meta_shader_context.shader = action.shader;
            } else if (action.kind == MenuActionKind::SizeChange) {
                m_canvas_divisor = action.size;
                m_pins.clear();
            } else if (action.kind == MenuActionKind::RestartAnimation) {
                m_start_time = GetTime();
            } else if (action.kind == MenuActionKind::ReloadShaders) {
                setupMetaShaders(m_meta_shader_context);
                m_menu.setup(m_meta_shader_context);
            }
        }

        computeSizes();

        updateCamera();


        // don't updated pins
        if (!m_menu.animationRunning())
            return;

        //
        // Run Meta Shader on each pin
        //
        float time = float(GetTime() - m_start_time);
        updateContextState(m_meta_shader_context, m_canvas_width, m_canvas_height, time);
        
        Vector2 pin;
        for (int y = 0; y != m_canvas_height; ++y) {
            pin.y = float(y);
            for (int x = 0; x != m_canvas_width; ++x) {
                int index = y * m_canvas_width + x;
                pin.x = float(x);
                m_pins[index] = clampTo(m_meta_shader_context.shader.function(m_meta_shader_context, index, pin, m_pins[index]), 0.0f, 1.0f);
            }
        }
    }

    void PinWorld::computeSizes() {
        m_window_width = float(GetScreenWidth());
        m_window_height = float(GetScreenHeight());

        //
        // initialization code
        //
        if (m_pins.empty()) {
            m_canvas_width = CANVAS_WIDTH / m_canvas_divisor;
            m_canvas_height = CANVAS_HEIGHT / m_canvas_divisor;
            TraceLog(LOG_DEBUG, "Canvas=%dx%d", m_canvas_width, m_canvas_height);

            int total = m_canvas_width * m_canvas_height;
            m_pins.resize(total, 0.0f);

            float x_start = m_canvas_width * m_pin_size_with_margins / 2.0f;
            x_start -= m_pin_size_with_margins / 2.0f; // center the first pin

            float y_start = m_canvas_height * m_pin_size_with_margins / 2.0f;
            y_start -= m_pin_size_with_margins / 2.0f; // center the first pin

            // Regenerate mesh
            UnloadMesh(m_pin_mesh);
            m_pin_mesh = GenMeshCube(m_pin_size, m_pin_height, m_pin_size);

            UnloadMaterial(m_pin_material);
            m_pin_material = loadPinMaterial(
                m_pin_mesh, m_pins,
                m_canvas_width, m_canvas_height, 
                x_start, y_start, 
                m_pin_size_with_margins
            );

            float max_dimension = maximum(m_canvas_width, m_canvas_height) * m_pin_size_with_margins;
            m_camera.position = { 0.0f, max_dimension / 2.0f, max_dimension / 1.5f };  // Camera position
            m_start_time = GetTime();
        }
    }

    void PinWorld::updateCamera() {
        bool u_k = m_menu.isMoveUpPressed();
        bool d_k = m_menu.isMoveDownPressed();
        bool l_k = m_menu.isMoveLeftPressed();
        bool r_k = m_menu.isMoveRightPressed();
        bool f_k = m_menu.isMoveForwardPressed();
        bool b_k = m_menu.isMoveBackwardPressed();
        float mouse_wheel = GetMouseWheelMove();
        bool has_mouse_wheel = abs(mouse_wheel) > 0.0f;
        bool key_pressed = u_k || d_k || l_k || r_k || f_k || b_k || has_mouse_wheel;

        if (key_pressed) {
            //UpdateCamera(&m_camera);
            //TraceLog(LOG_DEBUG, "Up='%d' Down='%d' Left='%d' Right='%d' Forward='%d' Backward='%d'", u_k, d_k, l_k, r_k, f_k, b_k);
            const float camera_orbit_speed = 0.5f * GetFrameTime(); // degrees per second
            const float camera_move_speed = 0.25f;
            const float keypad_zoom_speed = 2.0f;
            const bool lockView = true;
            const bool rotateAroundTarget = true;

            float yaw = r_k ? camera_orbit_speed : (l_k ? -camera_orbit_speed : 0.0f);
            float pitch = u_k ? -camera_orbit_speed : (d_k ? camera_orbit_speed : 0.0f);
            float move = f_k ? -camera_move_speed : (b_k ? camera_move_speed : 0.0f);

            if (has_mouse_wheel)
                move = mouse_wheel * keypad_zoom_speed;

            CameraPitch(&m_camera, pitch, lockView, rotateAroundTarget, false);
            CameraYaw(&m_camera, yaw, rotateAroundTarget);

            CameraMoveToTarget(&m_camera, move);
        }

    }
}