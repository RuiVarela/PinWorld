#include "Lang.hpp"
#include "Text.hpp"
#include "Menu.hpp"

#include "raylib.h"
#include "raygui.h"


static bool HoldableGuiButton(Rectangle bounds, const char *text) {
	GuiButton(bounds, text);

	return (CheckCollisionPointRec(GetMousePosition(), bounds) && IsMouseButtonDown(MOUSE_LEFT_BUTTON));
}

namespace pw
{
	Menu::Menu() {
		m_size_string.clear();

		for (int i = 0; i != 4; ++i) {
			int divisor = 1 << i;

			if (!m_size_string.empty())
				m_size_string += ";";

			m_size_string += sfmt("%dx%d", CANVAS_WIDTH / divisor, CANVAS_HEIGHT / divisor);
		}
	}

	Menu::~Menu() {
	}

	void Menu::shutdown() {
	}

	void Menu::setup(MetaShaderContext& meta_shader_context) {
		m_available_shaders = meta_shader_context.shaders;

		m_selected_animation_string.clear();
		for (auto& current : m_available_shaders) {
			if (!m_selected_animation_string.empty())
				m_selected_animation_string += ";";
			m_selected_animation_string += current.name;
		}

		if (!m_started) {
			addShaderAction(m_selected_animation_active);
			addSizeAction(m_size_active);
		} else {

			m_selected_animation_active = 0;
			for (size_t i = 0; i != m_available_shaders.size(); ++i) 
				if (m_available_shaders[i].name == meta_shader_context.shader.name) 
					m_selected_animation_active = int(i);	
		}
		 
		m_menu_key = KEY_M;
		m_up_key = KEY_W;
		m_down_key = KEY_S;
		m_left_key = KEY_A;
		m_right_key = KEY_D;
		m_forward_key = KEY_Q;
		m_backward_key = KEY_E;

		m_next_shader_key = KEY_RIGHT;
    	m_next_shader_fast_key = KEY_DOWN;

        m_previous_shader_key = KEY_LEFT;
        m_previous_shader_fast_key = KEY_UP;


		show(true);
		TraceLog(LOG_INFO, "Press '%c' for control options", m_menu_key);
		m_started = true;
	}

	bool Menu::animationRunning() {
		return m_animation_running;
	}

	void Menu::show(bool value) {
		m_window_controls_active = value;
	}
	bool Menu::showing() {
		return m_window_controls_active;
	}

	bool Menu::isMoveUpPressed()		{ return m_up_pressed || IsKeyDown(m_up_key); }
	bool Menu::isMoveDownPressed()		{ return m_down_pressed || IsKeyDown(m_down_key); }
	bool Menu::isMoveLeftPressed()		{ return m_left_pressed || IsKeyDown(m_left_key); }
	bool Menu::isMoveRightPressed()		{ return m_right_pressed || IsKeyDown(m_right_key) || m_orbiting; }
	bool Menu::isMoveForwardPressed()	{ return m_forward_pressed || IsKeyDown(m_forward_key); }
	bool Menu::isMoveBackwardPressed()	{ return m_backward_pressed || IsKeyDown(m_backward_key); }

	void Menu::render() {

		if (IsKeyPressed(m_menu_key)) show(!m_window_controls_active);

		if (IsKeyPressed(m_next_shader_key)) addNextShaderAction(1);
		else if (IsKeyPressed(m_next_shader_fast_key)) addNextShaderAction(5);
		else if (IsKeyPressed(m_previous_shader_key)) addNextShaderAction(-1);
		else if (IsKeyPressed(m_previous_shader_fast_key)) addNextShaderAction(-5);


		std::string animation_text = "";
		for (auto& current : m_available_shaders) {
			if (!animation_text.empty())
				animation_text += ";";
			animation_text += current.name;
		}


		float square_size = 24.0f;
		float margin = square_size / 2.0f;
		float controls_width = 184.0f;

		Vector2 window_pos = { float(GetScreenWidth()) - controls_width - margin, margin };

		// raygui: controls drawing
		//----------------------------------------------------------------------------------
		if (m_window_controls_active)
		{
			{
				Rectangle r = { window_pos.x, window_pos.y, controls_width, 11.0f * square_size + 14.0f * margin };
				m_window_controls_active = !GuiWindowBox(r, "Controls");
			}

			//
			// Camera
			//
			float group_y = window_pos.y + 24.0f + margin;
			float group_height = 4.0f * square_size + 3.0f * margin;
			float group_width = 160.0f;
			float group_content_x = window_pos.x + 24.0f;
			float group_content_y = group_y + margin;
			float fullsize_button = 136.0f;

			{
				Rectangle r = { window_pos.x + margin, group_y, group_width, group_height };
				GuiGroupBox(r, "Camera");

				r = { group_content_x, group_content_y + square_size, square_size, square_size };
				m_left_pressed = HoldableGuiButton(r, "L");

				r = { group_content_x + 48.0f, group_content_y + square_size, square_size, square_size };
				m_right_pressed = HoldableGuiButton(r, "R");

				r = { group_content_x + 24.0f, group_content_y + 2.0f * square_size, square_size, square_size };
				m_down_pressed = HoldableGuiButton(r, "D");

				r = { group_content_x + 24.0f, group_content_y, square_size, square_size };
				m_up_pressed = HoldableGuiButton(r, "U");

				r = { group_content_x + 112.0f, group_content_y, square_size, square_size };
				m_forward_pressed = HoldableGuiButton(r, "F");

				r = { group_content_x + 88.0f, group_content_y + 2.0f * square_size, square_size, square_size };
				m_backward_pressed = HoldableGuiButton(r, "B");

				r = { group_content_x, group_content_y + 3.0f * square_size + margin, square_size, square_size };
				m_orbiting = GuiCheckBox(r, "Orbit", m_orbiting);
			}

			//
			// Options
			//
			group_y += group_height + margin;
			group_content_y = group_y + margin;
			group_height = 2.0f * square_size + 2.0f * margin;

			{
				Rectangle r = { window_pos.x + margin, group_y, group_width, group_height };
				GuiGroupBox(r, "Options");

				r = { group_content_x, group_content_y, fullsize_button, square_size };
				GuiLabel(r, "Size");
				r = { group_content_x, group_content_y + square_size, fullsize_button, square_size };

				int old_size_active = m_size_active;
				m_size_active = GuiComboBox(r, m_size_string.c_str(), m_size_active);
				if (old_size_active != m_size_active)
					addSizeAction(m_size_active);
			}


			//
			// Animation
			//
			group_y += group_height + margin;
			group_content_y = group_y + margin;
			group_height = 4.0f * square_size + 5.0f * margin;

			{
				Rectangle r = { window_pos.x + margin, group_y, group_width, group_height };
				GuiGroupBox(r, "Animation");

				r = { group_content_x, group_content_y, square_size, square_size };
				m_animation_running = GuiCheckBox(r, "Running", m_animation_running);

				float item = 1.0f;
				r = { group_content_x, group_content_y + item * square_size + item * margin, fullsize_button, square_size };
				if (GuiButton(r, "Restart")) {
					m_actions.push_back({ MenuActionKind::RestartAnimation });
				}

				item += 1.0f;
				r = { group_content_x, group_content_y + item * square_size + item * margin, fullsize_button, square_size };
				if (GuiButton(r, "Reload")) {
					m_actions.push_back({ MenuActionKind::ReloadShaders });
				}

				item += 1.0f;
				r = { group_content_x, group_content_y + item * square_size + item * margin, fullsize_button, square_size };
				int old_selected_animation = m_selected_animation_active;
				m_selected_animation_active = GuiComboBox(r, animation_text.c_str(), m_selected_animation_active);
				if (m_selected_animation_active != old_selected_animation)
					addShaderAction(m_selected_animation_active);
			}
		}
	}

	void Menu::addShaderAction(int index) {
		MenuAction action;
		action.kind = MenuActionKind::ShaderChange;
		action.shader = m_available_shaders[index];
		m_actions.push_back(action);
	}

	void Menu::addNextShaderAction(int offset) {
		int next = (m_available_shaders.size() + (m_selected_animation_active + offset)) % m_available_shaders.size(); 

		//TraceLog(LOG_INFO, "Change changer '%d' -> '%d'", m_selected_animation_active, next);
		m_selected_animation_active = next;
		addShaderAction(next);
	}

	void Menu::addSizeAction(int index) {
		MenuAction action;
		action.kind = MenuActionKind::SizeChange;
		action.size = (1 << index);
		
		m_actions.push_back(action);
	}

	MenuActions Menu::takeActions() {
		MenuActions actions;
		std::swap(actions, m_actions);
		return actions;
	}
}