#ifndef WMA_AUDIO_IAUDIO_DEVICE_HPP
#define WMA_AUDIO_IAUDIO_DEVICE_HPP

#include "../core/Types.hpp"
#include "AudioTypes.hpp"

namespace wma {

    /**
     * @brief Abstract base interface for audio output devices.
     *
     * The audio counterpart to IWindowManager: it owns the platform handle and
     * the thread that drives it, and nothing above it needs to know whether the
     * samples end up in ALSA, SDL3 or nowhere at all. Consumers get one job —
     * fill a buffer of floats on demand, through setMixCallback().
     *
     * This interface deliberately stops at raw playback. Decoding files,
     * mixing several sounds together and computing 3D pan/attenuation are
     * platform-independent maths that belong in the engine on top (Aura3D's
     * AudioEngine), not in the platform layer.
     *
     * Lifecycle: open() -> setMixCallback() -> start() ... stop() -> close().
     * The destructor performs stop()/close() itself, so a device that goes out
     * of scope mid-playback is safe.
     *
     * @note Instances are not thread-safe. Drive one from a single thread; the
     *       mix callback is the sole exception and runs on the audio thread by
     *       design.
     */
    class IAudioDevice {
    public:
        virtual ~IAudioDevice() = default;

        /**
         * @brief Acquire the platform device described by @p config.
         *
         * @return WmaCode::Ok when the device is open and ready to start.
         *         WmaCode::Error when it is unavailable — no hardware, no
         *         sound server, an ALSA name that does not resolve, or a
         *         format the device refuses. This is an expected outcome
         *         rather than an exception: createAudioDevice() reacts to it by
         *         degrading to the next backend, exactly as a renderer falls
         *         back from Vulkan to software.
         *
         * The granted configuration is readable through getConfig() afterwards
         * and may differ from @p config.
         */
        [[nodiscard]] virtual WmaCode open(const AudioDeviceConfig& config) = 0;

        //! Release the platform device. Stops playback first if running. Safe to
        //! call on an already-closed device.
        virtual void close() noexcept = 0;

        /**
         * @brief Begin pulling samples from the mix callback.
         *
         * @return WmaCode::Ok once the device is running, WmaCode::Error if it
         *         is not open or the platform refused to start it.
         *
         * @note Without a callback installed the device runs and plays silence
         *       rather than failing — a valid state for a game that has not
         *       loaded any audio yet.
         */
        [[nodiscard]] virtual WmaCode start() = 0;

        //! Halt playback and guarantee the mix callback is no longer executing
        //! by the time this returns. Safe to call when already stopped.
        virtual void stop() noexcept = 0;

        //! Whether the device is currently pulling samples.
        [[nodiscard]] virtual bool isRunning() const noexcept = 0;

        /**
         * @brief Install the callback the device pulls samples from.
         *
         * @warning Call this while stopped. Replacing the callback of a running
         *          device races with the audio thread already executing it.
         */
        virtual void setMixCallback(AudioMixCallback callback) = 0;

        /**
         * @brief The configuration the device actually granted.
         *
         * Valid only after a successful open(); the sample rate and channel
         * count reported here are what the hardware accepted, which is what a
         * mixer must resample and pan against.
         */
        [[nodiscard]] virtual const AudioDeviceConfig& getConfig() const noexcept = 0;

        //! Which platform API this device drives.
        [[nodiscard]] virtual AudioBackend getBackendType() const noexcept = 0;

    protected:
        IAudioDevice() = default;
    };

} // namespace wma

#endif // WMA_AUDIO_IAUDIO_DEVICE_HPP
