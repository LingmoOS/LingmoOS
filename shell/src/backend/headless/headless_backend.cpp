// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/headless/headless_backend.h"

#include "compositor/compositor.h"
#include "compositor/output.h"
#include "core/main_loop.h"
#include "wayland/server_util.h"

namespace Lingmo::Backend {

HeadlessBackend::HeadlessBackend(compositor::Compositor *compositor, int outputCount)
    : m_compositor(compositor)
    , m_outputCount(outputCount)
    , m_clock(core::mainLoop())
{
}

std::vector<std::unique_ptr<compositor::Output>>
HeadlessBackend::createOutputs(compositor::Output *primary)
{
    primary->pixelSize = outputSize();
    primary->physicalSizeMm = base::Size(509, 286);
    primary->model = "headless";
    primary->name = "Lingmo-1";
    primary->refreshMilliHz = 60000;
    primary->preferred = true;

    std::vector<std::unique_ptr<compositor::Output>> additional;
    for (int index = 1; index < m_outputCount; ++index) {
        auto model = std::make_unique<compositor::Output>();
        model->pixelSize = primary->pixelSize;
        model->physicalSizeMm = primary->physicalSizeMm;
        model->refreshMilliHz = primary->refreshMilliHz;
        model->preferred = true;
        model->name = "Lingmo-" + std::to_string(index + 1);
        model->model = "headless-" + std::to_string(index + 1);
        additional.push_back(std::move(model));
    }
    return additional;
}

void HeadlessBackend::startClock()
{
    m_clock.onTimeout = [this] {
        const uint64_t generation = ++m_generation;
        aboutToPresent();
        m_compositor->frameSubmitted(generation);
        m_compositor->frameDrawn(Lingmo::Wayland::nowInMilliseconds(), generation);
    };
    m_clock.start(std::chrono::milliseconds(16));
}

} // namespace Lingmo::Backend
