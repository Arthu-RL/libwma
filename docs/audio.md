# Audio

Raw playback only. Decoding, mixing several sounds and 3D pan/attenuation are
platform-independent maths and belong in the engine above this.

## Open and play

```cpp
#include <wma/wma.hpp>

wma::AudioDeviceConfig config{
    .sampleRate      = 48000,
    .channelCount    = 2,
    .framesPerBuffer = 1024,   // ~21 ms at 48 kHz
};

auto device = wma::openAudioDevice(config);

device->setMixCallback([](std::span<f32> out) {
    for (f32& sample : out) sample = 0.0f;
});

if (device->start() != wma::WmaCode::Ok) { /* platform refused */ }
```

`openAudioDevice()` degrades through the compiled-in backends and ends at
`Null`, which cannot fail — it **never returns null and never throws** on a
machine with no sound card. You get a silent device instead of an error path.

## Read back what you got

The hardware negotiates. Ask, don't assume:

```cpp
const auto& granted = device->getConfig();   // sampleRate/framesPerBuffer may differ
```

A mixer that hardcodes the requested rate plays at the wrong pitch on a device
that refused it.

## The mix callback

Runs on the audio thread under a hard real-time deadline.

- **Fill every element.** The buffer is not pre-zeroed; returning early replays
  the previous period as a stutter.
- **No allocation, no locks, no I/O.** Any unbounded pause is an audible dropout.
- Span is exactly `frames * channelCount`, interleaved `f32`, nominally
  `[-1, 1]`. Nothing clamps — out-of-range values reach the hardware as clipping.

Swapping the callback on a running device is safe: it is handed over through
`MixCallbackSlot`, which swaps on the audio thread and frees the replaced one on
yours. The swap lands within a period or two, so `stop()` first if you need an
exact cutover. Don't call it from inside the callback.

## Lifecycle

```
open() -> setMixCallback() -> start() ... stop() -> close()
```

`stop()` guarantees the callback is no longer executing when it returns. The
destructor does both, so a device going out of scope mid-playback is fine.

## Backends

| | |
|---|---|
| `Alsa` | Desktop Linux default. One hop fewer than SDL to the same hardware. |
| `Sdl3` | Everything else — Android, WASM, Apple. Resolves to AAudio/OpenSL ES, Web Audio, CoreAudio. |
| `Null` | Always present. Accepts a config, plays nothing. |

```cpp
auto device = wma::openAudioDevice(config, wma::AudioBackend::Sdl3);   // override
const char* name = wma::audioBackendName(device->getBackendType());
```

`config.deviceName` is ALSA-only (`"default"`, `"plughw:0,0"`, `"pipewire"`).
Prefer `plughw` over `hw`: samples are handed over as `f32` and a raw `hw` device
that cannot take float will fail to open.

## Platform notes

**WASM** — browsers refuse audio until the page sees a user gesture. `start()`
reports success and stays suspended; call it again from an input handler. Repeat
calls are harmless.

**SDL** — the window manager calls `SDL_Quit()` when its last window dies, which
quits every subsystem. Destroy audio devices *before* the last window.
