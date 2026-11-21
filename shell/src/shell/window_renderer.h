// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <vector>

#include "compositor/compositor.h"

namespace Lingmo::Shell {

class ShellWindow;

// Renders the shell windows that asked for a frame, all in one pass at the
// start of the next output frame, on the renderer's context, as ui::Compositor
// draws every views::Widget layer of a frame in one go before viz composites.
// Each window's pixels go to its surface as a commit, so they reach the scene
// and its damage tracking the way a client's buffer does.
class WindowRenderer : public compositor::ContentProducer
{
public:
    explicit WindowRenderer(compositor::Compositor *compositor);
    WindowRenderer(const WindowRenderer &) = delete;
    WindowRenderer &operator=(const WindowRenderer &) = delete;
    ~WindowRenderer() override;

    // `window` is rendered at the next output frame, which this asks for.
    void scheduleRender(ShellWindow *window);
    // Forgets `window`, which is going away or hidden.
    void cancel(ShellWindow *window);
    bool isScheduled(const ShellWindow *window) const;

    // compositor::ContentProducer:
    bool hasPendingContent(const base::Rect &viewport) const override;
    void produceContent(render::Renderer &renderer) override;

    // Passes that rendered anything, and the windows the last one rendered.
    uint64_t passCount() const { return m_passCount; }
    const std::vector<const ShellWindow *> &lastPass() const { return m_lastPass; }

private:
    compositor::Compositor *m_compositor;
    std::vector<ShellWindow *> m_scheduled;
    // The windows of the pass in progress not rendered yet.
    std::vector<ShellWindow *> m_pass;
    std::vector<const ShellWindow *> m_lastPass;
    uint64_t m_passCount = 0;
};

} // namespace Lingmo::Shell

