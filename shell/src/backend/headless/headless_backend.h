// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_HEADLESS_BACKEND_H_
#define LINGMO_BACKEND_HEADLESS_BACKEND_H_

#include <memory>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"

namespace Lingmo {

namespace Compositor {
class Compositor;
struct Output;
} // namespace compositor

namespace Backend {

// Stands in for a display: synthesizes outputs and a presentation clock so the
// compositor and real toolkit clients run with no hardware.
//
// This is deliberately not a DrmBackend sibling. There is no shared interface to
// implement: DisplayRuntime and the topology coordinator take a DrmBackend and
// already treat a null one as "no hardware", which is exactly what headless is.
class HeadlessBackend
{
public:
    static base::Size outputSize() { return base::Size(1920, 1080); }

    HeadlessBackend(compositor::Compositor *compositor, int outputCount);
    HeadlessBackend(const HeadlessBackend &) = delete;
    HeadlessBackend &operator=(const HeadlessBackend &) = delete;

    // Describes the primary output and returns any further ones. A toolkit
    // client with no output has no screen to place itself on, so headless mode
    // would be untestable with real toolkit clients without these.
    std::vector<std::unique_ptr<compositor::Output>> createOutputs(compositor::Output *primary);

    // Starts the substitute vblank. Nothing presents frames without a display,
    // and a client that never gets a frame callback stops drawing after its
    // first buffer, so a plain 60 Hz timer stands in for the flip.
    void startClock();

    // Emitted before the compositor is told, so session state settles first.
    base::Signal<> aboutToPresent;

private:
    compositor::Compositor *const m_compositor;
    const int m_outputCount;
    base::Timer m_clock;
    uint64_t m_generation = 0;
};

} // namespace backend
} // namespace Lingmo

#endif // LINGMO_BACKEND_HEADLESS_BACKEND_H_
