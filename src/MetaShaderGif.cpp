#include "MetaShader.hpp"

#include "raymath.h"
#include "external/stb_image.h"

namespace pw {

    struct Frame {
        int w = 0;
        int h = 0;
        float ts = 0.0f;
        float duration = 0.0f;
        std::vector<float> data;

        float get_pixel(int x, int y) {
            return data[y * w + x];
        }

        // relative coordinates
        float sample_nn(float x, float y) {
            int ix = int(x * (w - 1));
            int iy = int(y * (h - 1));

            return get_pixel(ix, iy);
        }

        Frame() 
            : w(0), h(0), ts(0)
        { }

        Frame(Frame && o) noexcept {
            w = o.w;
            h = o.h;
            ts = o.ts;
            duration = o.duration;
            data = std::move(o.data);
        }
    private:
        //
        // don't allow automatic memory copies
        //
        Frame(const Frame& o) = delete;
        Frame& operator=(const Frame& o) = delete;
    };

    // info that is constat during a frame run
    struct MetaShaderGif {

        MetaShaderGif() {
            clean();
        }

        void clean() {
            total_time = 0.0;
            active_frame = -1;
            frames.clear();
            active_shader_name.clear();
        }

        void grayscaleFrame(stbi_uc* data, Frame& frame) {
            for (int y = 0; y != frame.h; ++y) {
                for (int x = 0; x != frame.w; ++x) {
                    stbi_uc* source_pixel = data + (y * frame.w + x) * 4;

                    float r = float(*(source_pixel + 0)) / 255.0f;
                    float g = float(*(source_pixel + 1)) / 255.0f;
                    float b = float(*(source_pixel + 2)) / 255.0f;
                    float a = float(*(source_pixel + 3)) / 255.0f;

                    float value = 0.2126f * r + 0.7152f * g + 0.0722f * b;
                    value *= a;

                    float* dst_pixel = frame.data.data() + (y * frame.w + x);
                    *dst_pixel = clampTo(value, 0.0f, 1.0f);
                }
            }
        }

        void loadGif(std::string const& name) {
            clean();

            if (!file_mapping.contains(name)) return;

            std::vector<uint8_t> file_data;
            if (!readRawBinary(file_mapping[name], file_data)) return;

            int* delays = nullptr;
            int x, y, z, comp;

            stbi_uc* gif = stbi_load_gif_from_memory(file_data.data(), file_data.size(), &delays, &x, &y, &z, &comp, 0);
            if (!gif || !delays) return;

            size_t frame_size = size_t(x) * y * comp;
            total_time = 0.0f;
            for (int f = 0; f != z; ++f) {
                float delay = delays[f] / 1000.f;
                if (delay < 0.0001) 
                    delay = 0.1;

                Frame frame;
                frame.w = x;
                frame.h = y;
                frame.duration = delay;
                frame.ts = total_time;
                frame.data.resize(size_t(x * y));

                grayscaleFrame(gif + frame_size * f, frame);

                frames.push_back(std::move(frame));
                total_time += frame.duration;
            }

            free(delays);
            stbi_image_free(gif);
        }

        void advanceTime(float time) {
            float modulated = fmod(time, total_time);
            for (size_t i = 0; i != frames.size(); ++i) {
                int index = (int(frames.size()) + active_frame + i) % frames.size();

                Frame const& f = frames[index];
                if (modulated >= f.ts && modulated < (f.ts + f.duration)) {
                    active_frame = index;
                    break;
                }
            }
        }

        void letterbox() {
            if (frames.empty()) return;

            // Compute source/destination ratios.
            Vector2 src_size = { float(frames[0].w), float(frames[0].h) };
            Vector2 dst_size = size;

            if (((src_size.x * dst_size.y) / src_size.y) <= dst_size.x) {
                // Column letterboxing ("pillar box")
                letterbox_region.x  = (dst_size.y * src_size.x) / src_size.y;
                letterbox_region.y = dst_size.y;
            } else {
                // Row letterboxing.
                letterbox_region.x  = dst_size.x;
                letterbox_region.y = (dst_size.x * src_size.y) / src_size.x;
            }

            letterbox_start = Vector2Scale(Vector2Subtract(dst_size, letterbox_region), 1.0f / 2.0f);
            letterbox_end = Vector2Add(letterbox_start, letterbox_region);
        }


        //
        // data
        //
        Vector2 size{};      // size of the canvas

        Vector2 letterbox_start{};
        Vector2 letterbox_end{};
        Vector2 letterbox_region{};

        std::map<std::string, std::string> file_mapping;

        std::string active_shader_name;
        std::vector<Frame> frames;
        float total_time;
        int active_frame;
    };


    static bool ensureShader(MetaShaderContext& msc) {
        if (msc.gif->active_shader_name != msc.shader.name) {
            msc.gif->loadGif(msc.shader.name);
        }

        msc.gif->active_shader_name = msc.shader.name;

        return !msc.gif->frames.empty();
    }

    void updateGifContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time) {
        ensureShader(context);

        context.gif->size = { float(canvas_width), float(canvas_height) };
        context.gif->advanceTime(time);
        context.gif->letterbox();
    }

    static float gifShader(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        if (msc.gif->active_frame < 0)
            return 0.0;

        if ( (pin_pos.x < msc.gif->letterbox_start.x || pin_pos.x > msc.gif->letterbox_end.x) ||
             (pin_pos.y < msc.gif->letterbox_start.y || pin_pos.y > msc.gif->letterbox_end.y) )
            return 0.0f;

        Vector2 relative_pos = Vector2Divide(Vector2Subtract(pin_pos, msc.gif->letterbox_start), msc.gif->letterbox_region);

        Frame& frame = msc.gif->frames[msc.gif->active_frame];
        return frame.sample_nn(relative_pos.x, relative_pos.y);
    }

    //
    // gif Meta shaders
    //
    void setupGifMetaShaders(MetaShaderContext& context) {
        context.gif = std::make_shared<MetaShaderGif>();

        std::vector<std::string> files = platformShadersFiles("gif");

		for (size_t i = 0; i != files.size(); ++i) {
			std::string filepath = files[i];
			std::string file = getSimpleFileName(files[i]);
			std::string name = getNameLessExtension(file);

            context.gif->file_mapping[name] = filepath;

            context.shaders.push_back({ name, gifShader });
        }
    }
}