namespace Lingmo::base { class EventLoop; }
namespace base = Lingmo::base;

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gpu/egl_context.h"

#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <malloc.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <vector>
#include <wayland-egl.h>

#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/gpu/ganesh/gl/egl/GrGLMakeEGLInterface.h"

#include "base/event_loop.h"
#include "gpu/program_cache.h"

namespace Lingmo::ui::gpu {

// How long after the last frame a window counts as idle.
static constexpr auto kIdleDelay = std::chrono::seconds(5);

// One config with alpha serves every window: popups are translucent around
// their rounded body, and opaque windows say so with their opaque region.
static EGLConfig chooseConfig(EGLDisplay display)
{
    const EGLint attributes[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE,
    };
    EGLint count = 0;
    if (!eglChooseConfig(display, attributes, nullptr, 0, &count) || count == 0)
        return nullptr;

    std::vector<EGLConfig> configs(count);
    eglChooseConfig(display, attributes, configs.data(), count, &count);
    for (EGLConfig config : configs) {
        EGLint alpha = 0;
        EGLint red = 0;
        eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &alpha);
        eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red);
        if (alpha == 8 && red == 8)
            return config;
    }
    return configs.front();
}

std::unique_ptr<EglContext> EglContext::create(wl_display* display)
{
    std::unique_ptr<EglContext> context(new EglContext);
    if (!context->initialize(display))
        return nullptr;
    return context;
}

std::unique_ptr<EglContext> EglContext::createInBackground(wl_display* display)
{
    std::unique_ptr<EglContext> context(new EglContext);
    EglContext* self = context.get();
    self->m_initializer = std::thread([self, display] {
        self->m_initialized = self->initialize(display);
        // The context is made current again on the thread that draws.
        if (self->m_initialized)
            eglMakeCurrent(self->m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglReleaseThread();
    });
    return context;
}

void EglContext::finishInitialization() const
{
    m_initializer.join();
    if (!m_initialized)
        std::exit(1);
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context);
}

bool EglContext::initialize(wl_display* display)
{
    m_display = eglGetPlatformDisplay(EGL_PLATFORM_WAYLAND_KHR, display, nullptr);
    if (m_display == EGL_NO_DISPLAY || !eglInitialize(m_display, nullptr, nullptr)) {
        fprintf(stderr, "lingmo-toolkit: cannot initialise EGL\n");
        return false;
    }
    eglBindAPI(EGL_OPENGL_ES_API);

    m_config = chooseConfig(m_display);
    if (!m_config) {
        fprintf(stderr, "lingmo-toolkit: no suitable EGL config\n");
        return false;
    }
    eglGetConfigAttrib(m_display, m_config, EGL_STENCIL_SIZE, &m_stencilBits);

    const char* extensions = eglQueryString(m_display, EGL_EXTENSIONS);
    const auto hasExtension = [extensions](const char* name) {
        return extensions && std::strstr(extensions, name);
    };
    m_hasBufferAge = hasExtension("EGL_EXT_buffer_age");
    if (hasExtension("EGL_KHR_swap_buffers_with_damage")) {
        m_swapBuffersWithDamage = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC>(
            eglGetProcAddress("eglSwapBuffersWithDamageKHR"));
    } else if (hasExtension("EGL_EXT_swap_buffers_with_damage")) {
        m_swapBuffersWithDamage = reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC>(
            eglGetProcAddress("eglSwapBuffersWithDamageEXT"));
    }

    const EGLint contextAttributes[] = {EGL_CONTEXT_MAJOR_VERSION, 3, EGL_NONE};
    m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttributes);
    if (m_context == EGL_NO_CONTEXT) {
        fprintf(stderr, "lingmo-toolkit: cannot create a GLES 3 context\n");
        return false;
    }

    // Skia needs a current context to probe the driver; no window exists yet.
    eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context);
    const auto glString = [](GLenum name) {
        const GLubyte* value = glGetString(name);
        return std::string(value ? reinterpret_cast<const char*>(value) : "");
    };
    m_programCache = std::make_unique<ProgramCache>(glString(GL_VENDOR) + "\n" + glString(GL_RENDERER) + "\n"
                                                    + glString(GL_VERSION));
    GrContextOptions options;
    options.fPersistentCache = m_programCache.get();
    m_skiaContext = GrDirectContexts::MakeGL(GrGLInterfaces::MakeEGL(), options);
    if (!m_skiaContext) {
        fprintf(stderr, "lingmo-toolkit: Skia cannot use the GLES context\n");
        return false;
    }
    return true;
}

EglContext::~EglContext()
{
    if (m_idleWatch)
        m_idleLoop->unwatch(m_idleWatch);
    if (m_idleFd >= 0)
        close(m_idleFd);
    if (m_initializer.joinable())
        m_initializer.join();
    if (m_skiaContext) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context);
        m_skiaContext->abandonContext();
        m_skiaContext.reset();
    }
    if (m_display != EGL_NO_DISPLAY) {
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (m_context != EGL_NO_CONTEXT)
            eglDestroyContext(m_display, m_context);
        eglTerminate(m_display);
    }
}

EGLSurface EglContext::createSurface(wl_egl_window* window)
{
    waitUntilReady();
    EGLSurface surface = eglCreatePlatformWindowSurface(m_display, m_config, window, nullptr);
    if (surface == EGL_NO_SURFACE) {
        fprintf(stderr, "lingmo-toolkit: cannot create an EGL window surface\n");
        return surface;
    }
    eglMakeCurrent(m_display, surface, surface, m_context);
    eglSwapInterval(m_display, 0);
    return surface;
}

void EglContext::destroySurface(EGLSurface surface)
{
    if (surface == EGL_NO_SURFACE)
        return;
    if (eglGetCurrentSurface(EGL_DRAW) == surface)
        eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, m_context);
    eglDestroySurface(m_display, surface);
}

bool EglContext::makeCurrent(EGLSurface surface)
{
    waitUntilReady();
    if (eglGetCurrentSurface(EGL_DRAW) == surface && eglGetCurrentContext() == m_context)
        return true;
    return eglMakeCurrent(m_display, surface, surface, m_context);
}

int EglContext::bufferAge(EGLSurface surface) const
{
    EGLint age = 0;
    if (!m_hasBufferAge || !eglQuerySurface(m_display, surface, EGL_BUFFER_AGE_EXT, &age))
        return 0;
    return age;
}

void EglContext::armIdleTimer()
{
    if (!m_idleLoop) {
        m_idleLoop = base::EventLoop::current();
        if (!m_idleLoop)
            return;
        m_idleFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        if (m_idleFd < 0)
            return;
        m_idleWatch = m_idleLoop->watch(m_idleFd, EPOLLIN, [this](uint32_t) {
            uint64_t expirations;
            if (read(m_idleFd, &expirations, sizeof(expirations)) <= 0)
                return;
            m_idleArmed = false;
            if (m_frames != m_framesWhenArmed) {
                armIdleTimer();
                return;
            }
            // Drawing frees most of what it allocates, and the allocator
            // keeps those pages until it is asked.
            malloc_trim(0);
        });
    }
    if (!m_idleWatch)
        return;
    itimerspec spec{};
    spec.it_value.tv_sec = kIdleDelay.count();
    timerfd_settime(m_idleFd, 0, &spec, nullptr);
    m_idleArmed = true;
    m_framesWhenArmed = m_frames;
}

bool EglContext::swapBuffers(EGLSurface surface, const std::vector<Rect>& damage, int height)
{
    ++m_frames;
    if (!m_idleArmed)
        armIdleTimer();
    if (!m_swapBuffersWithDamage || damage.empty())
        return eglSwapBuffers(m_display, surface);
    // EGL counts rows from the bottom.
    std::vector<EGLint> rects;
    rects.reserve(damage.size() * 4);
    for (const Rect& rect : damage) {
        rects.push_back(rect.x);
        rects.push_back(height - rect.y - rect.height);
        rects.push_back(rect.width);
        rects.push_back(rect.height);
    }
    return m_swapBuffersWithDamage(m_display, surface, rects.data(), EGLint(damage.size()));
}

} // namespace Lingmo::ui::gpu
