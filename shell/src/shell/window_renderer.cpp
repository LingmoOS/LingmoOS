// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/window_renderer.h"

#include <algorithm>

#include "render/renderer.h"
#include "shell/shell_window.h"
#include "toolkit/toolkit_host.h"

namespace Lingmo::Shell {

WindowRenderer::WindowRenderer(compositor::Compositor *compositor)
    : m_compositor(compositor)
{
    m_compositor->setContentProducer(this);
}

WindowRenderer::~WindowRenderer()
{
    if (m_compositor->contentProducer() == this)
        m_compositor->setContentProducer(nullptr);
}

void WindowRenderer::scheduleRender(ShellWindow *window)
{
    if (!isScheduled(window))
        m_scheduled.push_back(window);
    m_compositor->scheduleRepaint();
}

void WindowRenderer::cancel(ShellWindow *window)
{
    m_scheduled.erase(std::remove(m_scheduled.begin(), m_scheduled.end(), window),
                      m_scheduled.end());
    m_pass.erase(std::remove(m_pass.begin(), m_pass.end(), window), m_pass.end());
    m_lastPass.erase(std::remove(m_lastPass.begin(), m_lastPass.end(), window), m_lastPass.end());
}

bool WindowRenderer::isScheduled(const ShellWindow *window) const
{
    return std::find(m_scheduled.begin(), m_scheduled.end(), window) != m_scheduled.end();
}

bool WindowRenderer::hasPendingContent(const base::Rect &viewport) const
{
    return std::any_of(m_scheduled.begin(), m_scheduled.end(), [&](const ShellWindow *window) {
        return window->drawnBounds().intersects(viewport);
    });
}

void WindowRenderer::produceContent(render::Renderer &renderer)
{
    if (m_scheduled.empty())
        return;
    // Every scheduled window, whichever output's frame this is: the outputs
    // share the textures, and a window spanning two is drawn once. A render
    // that asks for the window's next frame, as an animation does, schedules
    // it for the next pass.
    m_pass.swap(m_scheduled);
    m_lastPass.clear();
    const uint64_t frameTimeNs = renderer.frameTime >= 0 ? uint64_t(renderer.frameTime * 1e6) : 0;
    toolkit::PanelBatch batch;
    while (!m_pass.empty()) {
        // Taken out first: rendering may destroy or hide any window of the pass.
        ShellWindow *window = m_pass.front();
        m_pass.erase(m_pass.begin());
        m_lastPass.push_back(window);
        window->produceFrame(renderer, frameTimeNs);
        // A window this render changed, such as a popup an animation moved,
        // is drawn in this pass too, or the frame shows the state it
        // committed at once without the pixels drawn for it.
        for (auto it = m_scheduled.begin(); it != m_scheduled.end();) {
            if (std::find(m_lastPass.begin(), m_lastPass.end(), *it) != m_lastPass.end()) {
                ++it;
                continue;
            }
            if (std::find(m_pass.begin(), m_pass.end(), *it) == m_pass.end())
                m_pass.push_back(*it);
            it = m_scheduled.erase(it);
        }
    }
    ++m_passCount;
}

} // namespace Lingmo::Shell
