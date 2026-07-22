#ifndef FRAMETIMER_H
#define FRAMETIMER_H

#include <chrono>
#include <ink/ink_base.hpp>

#ifndef __EMSCRIPTEN__
#include <thread>
#endif

#include "WindowFlags.hpp"

//! Clamp for the smallest measurable frame time (ms) so 1000/dt never divides by 0.
#define LIMIT_TARGET_FPS_TOLERANCE 1e-5
//! When within this many ms of the target, stop sleeping and busy-spin for accuracy.
#define LIMIT_SPIN_LOOP_DURATION 2e-3

namespace wma {

/**
 * @brief Measures per-frame delta time / FPS and (on desktop) paces the loop.
 *
 * Timing and rate-limiting are separated so the WASM run-loop can measure delta
 * every animation frame via endFrame(false) while the browser paces it, whereas
 * desktop calls endFrame(true) to sleep/spin down to the target frame time.
 */
class FrameTimer {
public:
    using clock = std::chrono::high_resolution_clock;

    explicit FrameTimer(WindowFlags& wFlags) noexcept : 
        windowFlags_(wFlags),
        lastFrameStart_(clock::now()),
        frameStart_(lastFrameStart_) {}

    void setTargetFPS(u32 fps) noexcept {
        targetFrameTime_ = std::chrono::duration<f64, std::milli>(fps > 0 ? 1000.0 / fps : 0.0);
    }

    //! Mark the start of a frame. Call before polling/updating.
    void beginFrame() noexcept { frameStart_ = clock::now(); }

    /**
     * @brief Close the frame: update deltaTime/fps and optionally pace the loop.
     * @param limit  When true (desktop) sleep/spin until targetFrameTime_ elapses.
     *               When false (WASM) only the timing is updated; the browser paces.
     */
    void endFrame(bool limit = true) {
        std::chrono::duration<f64, std::milli> elapsed = frameStart_ - lastFrameStart_;
        windowFlags_.deltaTime = INK_MAX(elapsed.count(), LIMIT_TARGET_FPS_TOLERANCE);
        windowFlags_.fps = 1000.0 / windowFlags_.deltaTime;
        lastFrameStart_ = frameStart_;

        if (!limit || targetFrameTime_.count() <= 0.0)
            return;

#ifndef __EMSCRIPTEN__
        std::chrono::duration<f64, std::milli> remaining = targetFrameTime_ - (clock::now() - frameStart_);

        if (remaining.count() > LIMIT_SPIN_LOOP_DURATION)
            std::this_thread::sleep_for(remaining - std::chrono::duration<f64, std::milli>(LIMIT_SPIN_LOOP_DURATION));

        while ((clock::now() - frameStart_) < targetFrameTime_) 
        {
            //! Busy-wait the final sub-millisecond for frame-time accuracy.
        }
#endif
    }

private:
    WindowFlags& windowFlags_;
    std::chrono::duration<f64, std::milli> targetFrameTime_{0.0}; //! 0 => unlimited
    clock::time_point lastFrameStart_;
    clock::time_point frameStart_;
};

} // namespace wma

#endif // FRAMETIMER_H
