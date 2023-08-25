#pragma once

#include "MetaShader.hpp"

namespace pw {

    constexpr int CANVAS_WIDTH = 320;
    constexpr int CANVAS_HEIGHT = 240;

    enum class MenuActionKind {
        None,
        ShaderChange,
        SizeChange,
        RestartAnimation,
        ReloadShaders
    };

    struct MenuAction {
        MenuActionKind kind = MenuActionKind::None;
        MetaShaderInfo shader = { "", nullptr };
        bool running = false;
        int size = 1;
    };
    using MenuActions = std::vector<MenuAction>;

    class Menu {
        public:
            Menu();
            ~Menu();

            void setup(MetaShaderContext& meta_shader_context);
            void render();
            void shutdown();

            bool isMoveUpPressed();
            bool isMoveDownPressed();
            bool isMoveLeftPressed();
            bool isMoveRightPressed();
            bool isMoveForwardPressed();
            bool isMoveBackwardPressed();

            bool animationRunning();

            void show(bool value = true);
            bool showing();

            MenuActions takeActions();
        private:
            bool m_started = false;

            int m_menu_key = 0;
            int m_up_key = 0;
            int m_down_key = 0;
            int m_left_key = 0;
            int m_right_key = 0;
            int m_forward_key = 0;
            int m_backward_key = 0;
            int m_next_shader_key = 0;
            int m_next_shader_fast_key = 0;
            int m_previous_shader_key = 0;
            int m_previous_shader_fast_key = 0;


            bool m_window_controls_active = true;
            bool m_left_pressed = false;
            bool m_right_pressed = false;
            bool m_down_pressed = false;
            bool m_up_pressed = false;
            bool m_forward_pressed = false;
            bool m_backward_pressed = false;
            bool m_orbiting = false;

            std::string m_selected_animation_string;
            int m_selected_animation_active = 0;
            bool m_animation_running = true;

            std::string m_size_string;
            int m_size_active = 1;

            MenuActions m_actions;
            MetaShadersInfo m_available_shaders;

            void addShaderAction(int index);
            void addNextShaderAction(int offset);
            void addSizeAction(int index);
    };

}