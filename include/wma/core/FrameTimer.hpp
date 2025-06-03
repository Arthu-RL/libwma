#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <thread>
#include <chrono>

#include "Types.hpp"
#include "WindowFlags.hpp"

#define LIMIT_TARGET_FPS_TOLERANCE 1e-5

namespace wma {

class FrameTimer {
public:
    WindowFlags& windowFlags_;
   std::chrono::duration<f64, std::milli> targetFrameTime_{16.666}; // Example: ~60 FPS (1000ms / 60 ≈ 16.66ms)

    FrameTimer(WindowFlags& wFlags) : windowFlags_(wFlags), lastFrameTime_(std::chrono::high_resolution_clock::now()) {
        // Initialize targetFrameTime_ based on desired FPS, e.g., setTargetFPS(60);
    }

    void setTargetFPS(unsigned int fps) {
        if (fps > 0) {
            targetFrameTime_ = std::chrono::duration<f64, std::milli>(1000.0 / fps);
        } else {
            targetFrameTime_ = std::chrono::duration<f64, std::milli>(0); // Effectively no limit or handle as error
        }
    }

    /**
     * @brief Call this at the START of your game loop.
     * Calculates deltaTime and FPS based on the previous frame.
     * Sets up lastFrameTime_ for the current frame's work measurement.
     */
    void updateDeltaTime() {
        std::chrono::time_point currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<f64, std::milli> elapsed = currentTime - lastFrameTime_;

        windowFlags_.deltaTime = INK_MAX(elapsed.count(), LIMIT_TARGET_FPS_TOLERANCE);

        windowFlags_.fps = 1000.0 / windowFlags_.deltaTime;

        lastFrameTime_ = currentTime;
    }

    /**
     * @brief Call this at the END of your game loop, after all processing and rendering.
     * Sleeps if the frame work completed faster than targetFrameTime_.
     */
    void limitFrameRate() {
        std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<f64, std::milli> frameWorkMs = std::chrono::duration<f64, std::milli>(currentTime - lastFrameTime_);

        if (frameWorkMs < targetFrameTime_ && targetFrameTime_.count() > 0.0) {
            std::chrono::milliseconds sleepDuration = std::chrono::duration_cast<std::chrono::milliseconds>(targetFrameTime_ - frameWorkMs);

            if (sleepDuration.count() > 0) {
                std::this_thread::sleep_for(sleepDuration);
            }
        }
    }

private:
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
};

}


#endif // FRAMETIMER_H
