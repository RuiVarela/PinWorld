#pragma once

#include "Lang.hpp"
#include "Text.hpp"
#include "MetaShader.hpp"
#include "Menu.hpp"


namespace pw {

	class PinWorld {
	public:
		PinWorld();
		~PinWorld();

		void setup();
		void run();
		void step();
		void shutdown();
	private:
		float m_window_width;
		float m_window_height;

		int m_canvas_divisor;
		int m_canvas_width;
		int m_canvas_height;
		float m_pin_size;
		float m_pin_size_with_margins;
		float m_pin_height;

		Camera3D m_camera;
		Menu m_menu;

		std::vector<float> m_pins;
		Mesh m_pin_mesh;
		Material m_pin_material;

		double m_start_time;

		// helpers for internal meta shader
		MetaShaderContext m_meta_shader_context;

		void render();
		void renderBackground();
		void renderPins();
		void update();
		void computeSizes();
		void updateCamera();
	};

}