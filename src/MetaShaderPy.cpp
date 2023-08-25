#include "pocketpy.h"

#include "MetaShader.hpp"

namespace pw {

	struct VMContext {
		std::string file;
		std::shared_ptr<pkpy::VM> vm;
		std::string error;
		PyObject* meta_shade = nullptr;

		PyObject* c_size = nullptr;
		PyObject* c_half_size = nullptr;
	};

	struct MetaShaderPy {
		std::map<std::string, VMContext> vmc;
	};

	static float python(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
		auto& vmc = msc.py->vmc[msc.shader.name];
		pkpy::VM* vm = vmc.vm.get();

		if (vmc.error.empty()) {
			PyVec2 pos(Vec2(pin_pos.x, pin_pos.y));

			try {
				PyObject* result = vm->call(vmc.meta_shade, VAR(pin_index), VAR(pos), VAR(pin_value));
				return CAST(float, result);
			} catch (pkpy::Exception& e) {
				std::string message = e.msg.c_str();
				vmc.error = message;
				TraceLog(LOG_ERROR, "python %s", e.msg.c_str());
			}
		}

		return 0.0f;
	}

	static void configureVM(VMContext& vmc) {
		pkpy::VM* vm = vmc.vm.get();

		vm->_stdout = [](pkpy::VM* vm, const pkpy::Str& text) {
			TraceLog(LOG_INFO, "VM %s", text.c_str());
		};

		vm->_stderr = [](pkpy::VM* vm, const pkpy::Str& text) {
			TraceLog(LOG_ERROR, "VM %s", text.c_str());
		};

		PyObject* mod = vmc.vm->new_module("pinworld");

		vm->bind_func<3>(mod, "smoothstep", PK_LAMBDA(VAR(smoothstep(CAST_F(args[0]), CAST_F(args[1]), CAST_F(args[2])))));
		vm->bind_func<1>(mod, "fract", PK_LAMBDA(VAR(fract(CAST_F(args[0])))));

	}

	void setupPyMetaShaders(MetaShaderContext& context) {
		context.py = std::make_shared<MetaShaderPy>();

		std::vector<std::string> files = platformShadersFiles("py");

		// generate vms
		for (size_t i = 0; i != files.size(); ++i) {
			std::string filepath = files[i];
			std::string file = getSimpleFileName(files[i]);
			std::string name = getNameLessExtension(file);

			std::string python_code;
			readRawText(filepath, python_code);

			VMContext vmc;
			vmc.file = file;
			vmc.vm = std::make_shared<pkpy::VM>();

			configureVM(vmc);

			vmc.vm->exec(python_code, file, EXEC_MODE);

			vmc.meta_shade = vmc.vm->_main->attr().try_get("meta_shade");
			vmc.c_size = vmc.vm->_main->attr().try_get("c_size");
			vmc.c_half_size = vmc.vm->_main->attr().try_get("c_half_size");
			PyObject* c_time = vmc.vm->_main->attr().try_get("c_time");

			if (vmc.meta_shade == nullptr) {
				TraceLog(LOG_ERROR, "%s > meta_shade function not found", file.c_str());
				continue;
			}

			if (vmc.c_size == nullptr) {
				TraceLog(LOG_ERROR, "%s > c_size not found", file.c_str());
				continue;
			}

			if (vmc.c_half_size == nullptr) {
				TraceLog(LOG_ERROR, "%s > c_half_size not found", file.c_str());
				continue;
			}

			if (c_time == nullptr) {
				TraceLog(LOG_ERROR, "%s > c_time not found", file.c_str());
				continue;
			}

			// if we reached this point, everything is ok and we can register this shader
			context.py->vmc[name] = vmc;
			context.shaders.push_back({ name, python });
		}
	}

	void updatePyContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time) {
		auto found = context.py->vmc.find(context.shader.name);
		if (found == context.py->vmc.end())
			return;

		VMContext& vmc = context.py->vmc[context.shader.name];
		pkpy::VM* vm = vmc.vm.get();		

		if (vmc.c_size != nullptr) {
			PyVec2& size = _CAST(PyVec2&, vmc.c_size);
			size.x = float(canvas_width);
			size.y = float(canvas_height);
		}

		if (vmc.c_half_size != nullptr) {
			PyVec2& half_size = _CAST(PyVec2&, vmc.c_half_size);
			half_size.x = float(canvas_width) / 2.0f;
			half_size.y = float(canvas_height) / 2.0f;
		}

		vm->_main->attr().set("c_time", VAR(time));
	}
}