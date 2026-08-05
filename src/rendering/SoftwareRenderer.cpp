#include "wma/rendering/SoftwareRenderer.hpp"

#include <algorithm>
#include <thread>
#include <vector>

#ifndef __EMSCRIPTEN__
#include <future>
#include <ink/ThreadPool.h>
#endif

namespace wma {

namespace {

    u32 hardwareThreadCount() noexcept {
#ifdef __EMSCRIPTEN__
        // Single-threaded WASM: no worker pool, so there is nothing to size.
        return 1;
#else
        const u32 count = std::thread::hardware_concurrency();
        return count > 0 ? count : 1;
#endif
    }

#ifndef __EMSCRIPTEN__
    //! Process-wide pool backing parallelFill(); every window's software
    //! rendering shares it, mirroring a single GPU's fixed set of cores.
    ink::ThreadPool& renderThreadPool() {
        static ink::ThreadPool pool(hardwareThreadCount());
        return pool;
    }
#endif

} // namespace

u32 softwareRenderWorkerCount() noexcept {
    return hardwareThreadCount();
}

namespace detail {

    void dispatchRowBands(const SoftwareFramebuffer& fb,
                           const std::function<void(i32 y, u32* row, i32 width)>& rowBand)
    {
        if (!fb.valid() || fb.height <= 0)
            return;

#ifdef __EMSCRIPTEN__
        // No thread pool on WASM (keeps the build free of -pthread /
        // SharedArrayBuffer / COOP-COEP requirements); walk every row inline.
        for (i32 row = 0; row < fb.height; ++row) {
            auto* rowPixels = reinterpret_cast<u32*>(
                static_cast<u8*>(fb.pixels) + static_cast<usize>(row) * fb.pitch);
            rowBand(row, rowPixels, fb.width);
        }
#else
        const i32 bands = std::min<i32>(static_cast<i32>(softwareRenderWorkerCount()), fb.height);
        const i32 rowsPerBand = fb.height / bands;
        const i32 remainder = fb.height % bands;

        auto& pool = renderThreadPool();
        std::vector<std::future<void>> futures;
        futures.reserve(static_cast<usize>(bands));

        i32 y = 0;
        for (i32 b = 0; b < bands; ++b)
        {
            const i32 bandRows = rowsPerBand + (b < remainder ? 1 : 0);
            const i32 yStart = y;
            const i32 yEnd   = y + bandRows;
            y = yEnd;

            futures.push_back(pool.submit([&fb, &rowBand, yStart, yEnd] {
                for (i32 row = yStart; row < yEnd; ++row) {
                    auto* rowPixels = reinterpret_cast<u32*>(
                        static_cast<u8*>(fb.pixels) + static_cast<usize>(row) * fb.pitch);
                    rowBand(row, rowPixels, fb.width);
                }
            }));
        }

        for (auto& f : futures)
            f.get();
#endif
    }

} // namespace detail

} // namespace wma
