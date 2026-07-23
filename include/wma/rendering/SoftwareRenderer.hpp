#ifndef WMA_RENDERING_SOFTWARE_RENDERER_HPP
#define WMA_RENDERING_SOFTWARE_RENDERER_HPP

#include <functional>

#include "wma/core/Types.hpp"

namespace wma {

//! Number of row-bands parallelFill() splits a frame into — one per hardware
//! thread (clamped to at least 1). Exposed for diagnostics/tuning only.
[[nodiscard]] u32 softwareRenderWorkerCount() noexcept;

namespace detail {

    //! Runs `rowBand(y, row, width)` once per row of `fb`, split into one
    //! contiguous row-band per hardware thread on the shared render pool, and
    //! blocks until every band completes. Not part of the stable API — use
    //! parallelFill() below.
    void dispatchRowBands(const SoftwareFramebuffer& fb,
                           const std::function<void(i32 y, u32* row, i32 width)>& rowBand);

} // namespace detail

/**
 * @brief Fill a software framebuffer in parallel across CPU cores.
 *
 * Splits the image into contiguous row-bands — one per hardware thread, the
 * same way a GPU spreads pixel work across its execution cores instead of a
 * single core walking every pixel — and runs them on a process-wide worker
 * pool shared by every window.
 *
 * @param fb    Framebuffer from IWindowManager::lockFramebuffer(); a no-op if invalid.
 * @param pixel Called as `pixel(x, y) -> u32` once per pixel, from worker
 *              threads, possibly concurrently on different threads for
 *              different pixels — it must not touch shared mutable state.
 *              The returned value is stored as that pixel's packed color.
 *
 * Blocks until the whole frame is written, so the framebuffer is ready to
 * present as soon as this returns.
 */
template <typename PixelFn>
void parallelFill(const SoftwareFramebuffer& fb, PixelFn&& pixel)
{
    detail::dispatchRowBands(fb, [&pixel](i32 y, u32* row, i32 width) {
        for (i32 x = 0; x < width; ++x)
            row[x] = static_cast<u32>(pixel(x, y));
    });
}

} // namespace wma

#endif // WMA_RENDERING_SOFTWARE_RENDERER_HPP
