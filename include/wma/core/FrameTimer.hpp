#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <ink/ink_base.hpp>

#include "WindowFlags.hpp"

#define LIMIT_TARGET_FPS_TOLERANCE 1e-5
#define LIMIT_SPIN_LOOP_DURATION 2e-3

namespace wma {

class FrameTimer {
public:
    WindowFlags& windowFlags_;

    // Ex: ~60 FPS (1000ms / 60 ≈ 16.66ms)
    std::chrono::duration<f64, std::milli> targetFrameTime_{0.0}; // Ilimitado

    FrameTimer(WindowFlags& wFlags)
        : windowFlags_(wFlags),
        lastFrameTime_(std::chrono::high_resolution_clock::now()),
        frameStartTime_(lastFrameTime_) {}

    void setTargetFPS(unsigned int fps) {
        if (fps > 0) {
            targetFrameTime_ = std::chrono::duration<f64, std::milli>(1000.0 / fps);
        } else {
            targetFrameTime_ = std::chrono::duration<f64, std::milli>(0.0);
        }
    }

    /**
     * @brief Chame no início do loop principal. Calcula deltaTime e FPS.
     */
    void updateDeltaTime() {
        frameStartTime_ = std::chrono::high_resolution_clock::now();
    }

    /**
     * @brief Chame no final do loop principal. Dorme se o frame terminar antes do tempo alvo.
     */
    void limitFrameRate() {
        std::chrono::duration<f64, std::milli> elapsed = frameStartTime_ - lastFrameTime_;

        windowFlags_.deltaTime = INK_MAX(elapsed.count(), LIMIT_TARGET_FPS_TOLERANCE);
        windowFlags_.fps = 1000.0 / windowFlags_.deltaTime;

        lastFrameTime_ = std::move(frameStartTime_);

        std::chrono::duration<f64, std::milli> remaining = targetFrameTime_ - (std::chrono::high_resolution_clock::now() - frameStartTime_);

        if (remaining.count() > LIMIT_SPIN_LOOP_DURATION) {
            std::this_thread::sleep_for(remaining-std::chrono::duration<f64, std::milli>(LIMIT_SPIN_LOOP_DURATION));
        }

        while ((std::chrono::high_resolution_clock::now() - frameStartTime_) < targetFrameTime_)
        {
            // Busy waiting
        }
    }

private:
    std::chrono::high_resolution_clock::time_point lastFrameTime_;
    std::chrono::high_resolution_clock::time_point frameStartTime_;

    std::mutex mt_;
    std::condition_variable cv_;
};

}

#endif // FRAMETIMER_H
