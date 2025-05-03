#pragma once
#include <chrono>

namespace velora
{
    class FpsCounter {
    public:
        FpsCounter(float smoothingFactor = 0.1f)
            : _ema_fps(0.0f), _smoothing(smoothingFactor), _initialized(false)
        {}

        void frame() {
            using namespace std::chrono;
            auto now = high_resolution_clock::now();

            if (_initialized) {
                auto delta = duration_cast<duration<float>>(now - _last_frame_time).count();
                float currentFps = 1.0f / delta;

                // EMA: new_value = alpha * current + (1 - alpha) * old
                _ema_fps = _smoothing * currentFps + (1.0f - _smoothing) * _ema_fps;
            } else {
                _ema_fps = 0.0f;
                _initialized = true;
            }

            _last_frame_time = now;
        }

        float getFPS() const {
            return _ema_fps;
        }

    private:
        std::chrono::high_resolution_clock::time_point _last_frame_time;
        float _ema_fps; // exponent moving average for smooth fps
        float _smoothing;
        bool _initialized;
    };
}