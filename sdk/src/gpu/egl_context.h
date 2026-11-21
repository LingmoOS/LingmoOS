// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <memory>
#include <thread>
#include <vector>

#include "include/core/SkRefCnt.h"

#include "ui/geometry.h"

class GrDirectContext;
struct wl_display;
struct wl_egl_window;

namespace Lingmo::base {
class EventLoop;
}

namespace Lingmo::ui::gpu {

class ProgramCache;

// One GLES context and its Skia context, shared by every window. Windows
// make their own EGL surface current before drawing.
class EglContext
{
public:
    static std::unique_ptr<EglContext> create(wl_display* display);
    // Loads the driver and probes it on a thread of its own while the caller
    // goes on; the first call that needs the context waits for it there, and
    // exits the process when there is none.
    static std::unique_ptr<EglContext> createInBackground(wl_display* display);
    ~EglContext();

    EglContext(const EglContext&) = delete;
    EglContext& operator=(const EglContext&) = delete;

    GrDirectContext* skiaContext() const
    {
        waitUntilReady();
        return m_skiaContext.get();
    }
    EGLDisplay display() const
    {
        waitUntilReady();
        return m_display;
    }
    EGLConfig config() const
    {
        waitUntilReady();
        return m_config;
    }
    int stencilBits() const
    {
        waitUntilReady();
        return m_stencilBits;
    }

    // Swaps never wait for a frame callback: windows pace themselves.
    EGLSurface createSurface(wl_egl_window* window);
    void destroySurface(EGLSurface surface);
    bool makeCurrent(EGLSurface surface);
    // How many frames ago the current back buffer was presented, 0 when its
    // content is unknown.
    int bufferAge(EGLSurface surface) const;
    // The damage is in device pixels from the top left of a buffer this tall.
    bool swapBuffers(EGLSurface surface, const std::vector<Rect>& damage, int height);

private:
    EglContext() = default;

    bool initialize(wl_display* display);
    void waitUntilReady() const
    {
        if (m_initializer.joinable())
            finishInitialization();
    }
    void finishInitialization() const;
    void armIdleTimer();

    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLConfig m_config = nullptr;
    EGLContext m_context = EGL_NO_CONTEXT;
    int m_stencilBits = 0;
    bool m_hasBufferAge = false;
    PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC m_swapBuffersWithDamage = nullptr;
    std::unique_ptr<ProgramCache> m_programCache;
    sk_sp<GrDirectContext> m_skiaContext;
    mutable std::thread m_initializer;
    bool m_initialized = false;
    // Armed by the first frame after a quiet spell, and checked when it
    // fires: a timer moved on every frame would cost a clock read in each
    // loop iteration.
    base::EventLoop* m_idleLoop = nullptr;
    int m_idleFd = -1;
    int m_idleWatch = 0;
    uint64_t m_frames = 0;
    uint64_t m_framesWhenArmed = 0;
    bool m_idleArmed = false;
};

} // namespace Lingmo::ui::gpu
