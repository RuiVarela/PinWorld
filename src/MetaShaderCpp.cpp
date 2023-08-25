#include "MetaShader.hpp"
#include "Text.hpp"

#include "raymath.h"

#include <filesystem>

namespace pw {

    // info that is constat during a frame run
    struct MetaShaderCpp {
        Vector2 size;                       // size of the canvas
        Vector2 half_size;                  // half size of the canvas
        float time;                         // time in seconds

        const float wave_cycles = 5.0f;
        const float wave_speed = 3.0f;
    };

    void updateContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time) {
        context.cpp->size = { float(canvas_width), float(canvas_height) };
        context.cpp->half_size = { canvas_width * 0.5f, canvas_height * 0.5f };
        context.cpp->time = time;

        updateGifContextState(context, canvas_width, canvas_height, time);
        updatePyContextState(context, canvas_width, canvas_height, time);
        updateWaterContextState(context, canvas_width, canvas_height, time);
    }

    static float swirl(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        Vector2 const& c_size = msc.cpp->size;
        float const& c_time = msc.cpp->time;

        float t = c_time * 0.25f;
        float out = 0.0f;

        Vector2 target_dir = Vector2Rotate({ 1.0f, 0.0f }, TWO_PI * t * 0.15f);
        target_dir = Vector2Normalize(target_dir);

        Vector2 pos_normalized = Vector2Divide(pin_pos, c_size);
        pos_normalized = Vector2Scale(pos_normalized, 2.0f);
        pos_normalized = Vector2SubtractValue(pos_normalized, 1.0f);
        

        float d = Vector2DotProduct(pos_normalized, target_dir);

        float displacement = d * TWO_PI * 3.5f;
        out = sin(t + displacement) * 0.5f + 0.5f;

        //float modulator = time;
        //float x_distance = smoothstep(0.0f, float(m_canvas_width), float(x)) * 2.0f - 1.0f;
        //7x_distance = repeatSpace(1.0f, x_distance + modulator);
        //x_distance = absolute(x_distance);

        // x_distance = (y == 0) ? 1.0f : 0.0f;

        return out;
    }

    static float cross(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        Vector2 const& c_size = msc.cpp->size; // canvas size
        float const& c_time = msc.cpp->time;

        float out = 0.0f;
        float size = 0.1f;

        // [-1.0f, 1.0f]
        Vector2 pos_normalized = Vector2SubtractValue(Vector2Scale(Vector2Divide(pin_pos, c_size), 2.0f), 1.0f);

        // draw 2 lines
        out += smoothstep(size, 0.0f, abs(pos_normalized.x));
        out += smoothstep(size, 0.0f, abs(pos_normalized.y));

        // make it move
        out *= fract(c_time * 0.5f);

        return out;
    }

    static float asteroid(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        float out = 0.0f;
        float size = 0.25f;

        // [-1.0f, 1.0f]
        Vector2 pos_normalized = Vector2SubtractValue(Vector2Scale(Vector2Divide(pin_pos, msc.cpp->size), 2.0f), 1.0f);

        float value = std::pow(std::abs(pos_normalized.x), 2.0f/3.0f) + std::pow(std::abs(pos_normalized.y), 2.0f / 3.0f) - 1.0f;
        out += smoothstep(size, 0.0f, abs(value));
        return out;
    }

    static float heart(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        float out = 0.0f;
        float size = 0.5f;

        // [-1.0f, 1.0f]
        Vector2 pos_normalized = Vector2SubtractValue(Vector2Scale(Vector2Divide(pin_pos, msc.cpp->size), 2.0f), 1.0f);

        // [-2.0f, 2.0f]
        pos_normalized = Vector2Scale(pos_normalized, 2.0f);

        float value = std::pow(pos_normalized.x, 2.0f) + std::pow(-pos_normalized.y - std::pow(std::pow(pos_normalized.x, 2.0f), 1.0f/3.0f), 2.0f) -1.0f;
        out += smoothstep(size, 0.0f, abs(value));
        return out;
    }


    static float ellipses(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        float out = 0.0f;
        float size = 0.5f;
        float speed = 0.04f;

        // [-1.0f, 1.0f]
        Vector2 pos_normalized = Vector2SubtractValue(Vector2Scale(Vector2Divide(pin_pos, msc.cpp->size), 2.0f), 1.0f);

  
        pos_normalized = Vector2Scale(pos_normalized, 10.0f);

        float x = pos_normalized.x + msc.cpp->time * speed;
        float y = pos_normalized.y + msc.cpp->time * speed;

        float value = sin(std::pow(x, 2.0f) + std::pow(y, 2.0f)) - cos(x * y);

        out += smoothstep(size, 0.0f, abs(value));

        return out;
    }

    static float circle(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        Vector2 const& c_half_size = msc.cpp->half_size; // canvas half size
        float const& c_time = msc.cpp->time;

        float out = 0.0f;
        float size = 0.3f;
        float radius = 0.7f;

        Vector2 pos_normalized = Vector2Scale(Vector2Subtract(pin_pos, c_half_size), 1.0f/ c_half_size.y);

        // draw 2 lines
        //out += smoothstep(size, 0.0f, abs(pos_normalized.x));
        float distance = Vector2Length(pos_normalized);

        out = smoothstep(size, 0.0f, abs(radius - distance));

        // make it move
        out = out * sin(c_time) * 0.5f + 0.5f;

        return out;
    }

    static float sinWave(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        float phase = (pin_pos.x / msc.cpp->size.x) * 2.0f * PI * msc.cpp->wave_cycles;
        return sin(phase + msc.cpp->time * msc.cpp->wave_speed) * 0.5f + 0.5f;
    }

	static float triangleWave(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
		const float relative_x = (pin_pos.x / msc.cpp->size.x);
		float phase = (relative_x * TWO_PI) * msc.cpp->wave_cycles;
		phase = fmodf(phase + msc.cpp->time * msc.cpp->wave_speed, TWO_PI);

		return 1.0f - fabs(-1.0f + (phase / PI));
	}

    static float sawWave(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        const float x_diff = 1.0f / msc.cpp->size.x;

        float t = int(msc.cpp->time * (msc.cpp->wave_speed * 2.0f) * TWO_PI);
        float value = fmodf((pin_pos.x + t) * x_diff * msc.cpp->wave_cycles, 1.0f);
        return value;
    }

    float whiteNoise() {
        constexpr int q = 15;
        constexpr float c1 = (1 << q) - 1;
        constexpr float c2 = ((int)(c1 / 3)) + 1;
        constexpr float c3 = 1.f / c1;

        float random = uniformRandom();
        return (2.f * ((random * c2) + (random * c2) + (random * c2)) - 3.f * (c2 - 1.f)) * c3;
    }

    static float randomData(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
         return uniformRandom() * 0.5f + 0.5f;
    }

    //
    // c++ Meta shaders
    //
    void setupMetaShaders(MetaShaderContext& context) {
        context.shaders.clear();

        setupWaterMetaShaders(context);
        setupGifMetaShaders(context);

        context.cpp = std::make_shared<MetaShaderCpp>();
        context.shaders.push_back({ "Sin Wave", sinWave });
        context.shaders.push_back({ "Triangle Wave", triangleWave });
        context.shaders.push_back({ "Saw Wave", sawWave });
        context.shaders.push_back({ "Swirl", swirl });
        context.shaders.push_back({ "Cross", cross });
        context.shaders.push_back({ "Asteroid", asteroid });
        context.shaders.push_back({ "Heart", heart });
        context.shaders.push_back({ "Ellipses", ellipses });
        context.shaders.push_back({ "Circle", circle });
        context.shaders.push_back({ "Random", randomData });

        setupPyMetaShaders(context);


        if (!context.shader.name.empty()) {
            std::string const& active = context.shader.name;
            bool found = std::any_of(context.shaders.begin(), context.shaders.end(), [&active](auto const& current) {  return current.name == active; });
            if (!found) {
                context.shader = context.shaders[0];
            }
        }
    }

    std::vector<std::string> platformShadersFiles(std::string const& kind) {
		std::vector<std::string> folders;

		
#if defined(__EMSCRIPTEN__)
		auto current = std::filesystem::path(executablePath()).parent_path();
		folders.push_back(mergePaths(current.string(), kind)); // win32
#elif defined(_DEBUG) || !defined(NDEBUG)
			// when debugging we should search the py files on the root of the code
			// not the packaged ones
			const int max_search = 4;
			int search = 0; // on mac search should find it at 3
			auto parent = std::filesystem::path(executablePath()).parent_path();;
			while (search < max_search) {
				if (!parent.has_parent_path()) 
					break;
					
				parent = parent.parent_path();
				std::string folder = mergePaths(parent.string(), kind);
				if (fileType(folder) == FileType::FileDirectory) {
					folders.push_back(folder);
					break;
				} 

				++search;
			}
#else
		auto current = std::filesystem::path(executablePath()).parent_path();
		folders.push_back(mergePaths(current.string(), kind)); // win32
		if (current.has_parent_path()) {
			auto parent = current.parent_path();
			folders.push_back(mergePaths(parent.string(), "Resources", kind)); // macosx
		}
#endif


        std::vector<std::string> files;

		// find candidate shaders
		for (auto const& shader_folder : folders) {
			TraceLog(LOG_DEBUG, "[%s] Checking folder %s", kind.c_str(), shader_folder.c_str());
			DirectoryContents contents = getDirectoryContents(shader_folder);
			for (auto const& file : contents) {
				std::string filepath = mergePaths(shader_folder, file);

				TraceLog(LOG_DEBUG, "[%s] Checking File %s", kind.c_str(), filepath.c_str());
						
				if (fileType(filepath) != FileType::FileRegular) continue;
				if (!endsWith(filepath, "." + kind)) continue;

				files.push_back(filepath);
			}
		}

		return files;
	}
}