#include "MetaShader.hpp"
#include "Text.hpp"

#include "raymath.h"

#include <filesystem>

// https://www.youtube.com/watch?v=PH9q0HNBjT4&list=PLFTSYFO3lrKw35oVgO_GzXbvu7medjsG6&index=4
// https://github.com/GarrettGunnell/Water/blob/main/Assets/Scripts/Water.cs

namespace pw {

    const float Meters = 10.0f;

    constexpr int WaveCount = 3;

    constexpr float MedianWavelength = 1.0f; // [0.0f, 3.0f]
    constexpr float WavelengthRange = 1.0f; // [0.0f, 2.0f]

    constexpr float MedianDirection = 0.0f; // [0.0f, 360.0f]
    constexpr float DirectionalRange = 180.0f; // [0.0f, 360.0f]

    constexpr float MedianAmplitude = 0.3f; // [0.0f, 3.0f]
    constexpr float MedianSpeed = 1.0f; // [0.0f, 2.0f]
    constexpr float SpeedRange = 0.1f; // [0.0f, 1.0f]

    // code ported from GarrettGunnell;
    enum class WaveType {
        Directional = 0,
        Circular,
    };

	struct Wave {
		Vector2 direction;
		Vector2 origin;
		float frequency;
		float amplitude;
		float phase;
		WaveType waveType;

        Wave() 
            :direction{1.0f, 0.0}, origin{0.0f, 0.0f}, frequency(1.0f), amplitude(1.0f), phase(0.0f), waveType(WaveType::Circular)
            { }

		Wave(float wavelength, float amplitude, float speed, float direction, WaveType waveType, Vector2 origin)
			:direction(Vector2Normalize({ cos(DEG2RAD * direction), sin(DEG2RAD * direction) })),
			origin(origin),
			frequency(2.0f / wavelength),
			amplitude(amplitude),
			phase(speed * std::sqrt(9.8f * 2.0f * PI / wavelength)),
			waveType(waveType)
		{ }

		Vector2 GetDirection(Vector2 const& p) {
			if (waveType == WaveType::Circular) {
				return Vector2Normalize(Vector2Subtract(p, origin));
			}

			return direction;
		}

        float GetWaveCoord(Vector2 const& p, Vector2 const& d) {
            if (waveType == WaveType::Circular) {
                Vector2 heading = Vector2Subtract(p, origin);
                return Vector2Length(heading);
            }

            return p.x * d.x + p.y * d.y;
        }

        float GetTime(float clock) {
            const float t = clock;
            return waveType == WaveType::Circular ? -t * phase : t * phase;
        }

		float Sine(Vector2 const& p, float clock) {
			Vector2 d = GetDirection(p);
			float xz = GetWaveCoord(p, d);

			return ::sin(frequency * xz + GetTime(clock)) * amplitude;
		}
	};



    struct MetaShaderWater {
        Vector2 size;                       // size of the canvas
        Vector2 half_size;                  // half size of the canvas
        float time;                         // time in seconds

        Wave waves[WaveCount];
    };

    void updateWaterContextState(MetaShaderContext& context, int canvas_width, int canvas_height, float time) {
        context.wtr->size = { float(canvas_width), float(canvas_height) };
        context.wtr->half_size = { canvas_width * 0.5f, canvas_height * 0.5f };
        context.wtr->time = time;
    }

    static float water(MetaShaderContext& msc, int const pin_index, Vector2 const& pin_pos, float const pin_value) {
        // normalized with correcte aspect [-x, -1.0] -> [+x, 1.0]
        Vector2 pos = Vector2Scale(Vector2Subtract(pin_pos, msc.wtr->half_size), 1.0f / msc.wtr->half_size.y);
        
        // scaled to be big
        pos = Vector2Scale(pos, Meters);


        float out = 0.0f;
        for (int i = 0; i != WaveCount; ++i) {
            out += msc.wtr->waves[i].Sine(pos, msc.wtr->time);
        }
        
        // scale back to 0.0 > 1.0
        out = out * 0.5f + 0.5f;
        return out;
    }

    void setupWaterMetaShaders(MetaShaderContext& context) {
        context.wtr = std::make_shared<MetaShaderWater>();

        constexpr  WaveType waveType = WaveType::Directional;
        constexpr float wavelengthMin = MedianWavelength / (1.0f + WavelengthRange);
        constexpr float wavelengthMax = MedianWavelength * (1.0f + WavelengthRange);
        constexpr float directionMin = MedianDirection - DirectionalRange;
        constexpr float directionMax = MedianDirection + DirectionalRange;
        constexpr float speedMin = maximum(0.01f, MedianSpeed - SpeedRange);
        constexpr float speedMax = MedianSpeed + SpeedRange;
        constexpr float ampOverLen = MedianAmplitude / MedianWavelength;


        for (int i = 0; i != WaveCount; ++i) {
            float wavelength = uniformRandomRange(wavelengthMin, wavelengthMax);
            float direction = uniformRandomRange(directionMin, directionMax);
            float amplitude = wavelength * ampOverLen;
            float speed = uniformRandomRange(speedMin, speedMax);
            Vector2 origin{ uniformRandomRange(-Meters, Meters), uniformRandomRange(-Meters, Meters) };
      
            context.wtr->waves[i] = Wave(wavelength, amplitude, speed, direction, waveType, origin);
        }

        context.shaders.push_back({ "Water", water });
    }
}