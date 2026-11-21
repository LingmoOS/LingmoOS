# Cadence

Cadence is a standalone, header-only C++20 library for deterministic, frame-driven animation. It has no window-system or toolkit dependency; applications supply frame timestamps and connect updates to their own rendering loop.

## First-version scope

- `Timeline`: chrono-typed duration, easing, loops, alternating playback, pause and resume; can read a supplied `Clock`.
- `Tween<T>`: timestamp-driven interpolation for arithmetic types, `Color`, `Point`, `Size`, and `Rect`, with a specialization point for custom types.
- `KeyframeTrack<T>`: validated normalized multi-segment sampling using the same interpolators and per-segment easing.
- `Spring`: closed-form critically damped scalar motion advanced by chrono frame delta, with initial velocity support.
- `ManualClock`: a small controllable clock for tests and adapters.

The API provides deterministic timing and value tracks, but intentionally leaves frame scheduling to the host. It does not yet provide GUI adapters, cancellation tokens, or automatic frame subscription.

## Build and test

```sh
cmake -S . -B build -DCADENCE_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Use

```cpp
#include <cadence/cadence.h>

using namespace std::chrono_literals;
Cadence::ManualClock clock;
Cadence::Tween<double> fade;
fade.setDuration(180ms);
fade.setEasing(Cadence::Easing::OutCubic);
fade.onValueChanged = [](double opacity) { /* update a view */ };
fade.setValue(0.0);
fade.animateTo(1.0, clock);

// Call once per rendered frame. The host owns frame scheduling.
clock.advance(16ms);
fade.advance(clock);
```

Use monotonic timestamps (or a monotonic `Clock`) and continue advancing a tween until `isRunning()` becomes false. Calling `animateTo()` again retargets from the tween's current value. `Spring::advance()` accepts any `std::chrono::duration` as elapsed time.

`KeyframeTrack<T>` offsets are finite, strictly increasing values in `[0, 1]`; a keyframe's easing controls the segment from that keyframe to the next. Sampling outside the track holds the nearest endpoint. Supply `Interpolator<T>` for custom value types. Easing functions may overshoot; integral and color interpolators clamp their result to the representable range.

## Install

```sh
cmake --install build --prefix /usr/local
```

Downstream CMake projects can use `find_package(Cadence CONFIG REQUIRED)` and link `Cadence::Cadence`.

## License

BSD-3-Clause; see [LICENSE](LICENSE).
