// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/egl_render_target.h"

#include <EGL/eglext.h>
#include <GLES3/gl3.h>

#include "core/logging.h"
#include "render/egl_context.h"
#include "base/region.h"

namespace Lingmo::Render {

EglRenderTarget::EglRenderTarget(EGLDisplay display, EGLSurface surface, gbm_surface *gbmSurface,
                                 const base::Size &size)
    : m_display(display)
    , m_surface(surface)
    , m_gbmSurface(gbmSurface)
    , m_size(size)
{
    const char *list = eglQueryString(display, EGL_EXTENSIONS);
    const std::string_view names = list ? list : "";
    const bool partialUpdate = hasExtension(names, "EGL_KHR_partial_update");
    m_bufferAge = partialUpdate || hasExtension(names, "EGL_EXT_buffer_age");
    if (partialUpdate)
        m_setDamageRegion = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(
                eglGetProcAddress("eglSetDamageRegionKHR"));
    const char *swapName = hasExtension(names, "EGL_KHR_swap_buffers_with_damage")
            ? "eglSwapBuffersWithDamageKHR"
            : hasExtension(names, "EGL_EXT_swap_buffers_with_damage")
            ? "eglSwapBuffersWithDamageEXT"
            : nullptr;
    if (swapName)
        m_swapBuffersWithDamage =
                reinterpret_cast<PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC>(eglGetProcAddress(swapName));
}

EglRenderTarget::~EglRenderTarget()
{
    if (m_surface != EGL_NO_SURFACE)
        eglDestroySurface(m_display, m_surface);
}

std::unique_ptr<EglRenderTarget> EglRenderTarget::createPbuffer(EGLDisplay display,
                                                                EGLConfig config, const base::Size &size)
{
    const EGLint attributes[] = {EGL_WIDTH, size.width(), EGL_HEIGHT, size.height(), EGL_NONE};
    const EGLSurface surface = eglCreatePbufferSurface(display, config, attributes);
    if (surface == EGL_NO_SURFACE)
        return nullptr;
    return std::make_unique<EglRenderTarget>(display, surface, nullptr, size);
}

std::unique_ptr<EglRenderTarget> EglRenderTarget::create(EGLDisplay display, EGLConfig config,
                                                         gbm_surface *surface, const base::Size &size)
{
    auto createPlatformWindowSurface = reinterpret_cast<PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC>(
            eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT"));

    EGLSurface eglSurface = EGL_NO_SURFACE;
    if (createPlatformWindowSurface) {
        eglSurface = createPlatformWindowSurface(display, config, surface, nullptr);
    } else {
        eglSurface = eglCreateWindowSurface(
                display, config, reinterpret_cast<EGLNativeWindowType>(surface), nullptr);
    }

    if (eglSurface == EGL_NO_SURFACE) {
        LM_CRITICAL(lingmoRender) << "Failed to create an EGL window surface";
        return nullptr;
    }

    auto target = std::make_unique<EglRenderTarget>(display, eglSurface, surface, size);

    return target;
}

base::Rect EglRenderTarget::prepareDamage(const base::Region &damage, const std::vector<base::Rect> &samples,
                                     const OutputRenderContext &context)
{
    m_damagePrepared = true;
    const base::Rect full(base::Point(), size());
    const auto &old = m_damageContext;
    if (old.logicalViewport != context.logicalViewport || old.scale != context.scale
        || old.transform != context.transform
        || old.nativeContentRect != context.nativeContentRect) {
        m_damageValid = false;
        m_damageHistory.reset();
    }
    m_damageContext = context;
    EGLint age = 0;
    if (m_bufferAge && !eglQuerySurface(m_display, eglSurface(), EGL_BUFFER_AGE_EXT, &age))
        age = 0;
    m_frameDamage = physicalDamage(damage, context, size());
    if (!m_damageValid) {
        age = 0;
        m_frameDamage = full;
    }
    // Repairing an old buffer must not turn unchanged content into future damage.
    base::Rect repaint = coverSamples(m_damageHistory.repair(m_frameDamage, age, full), samples,
                                 context, size());
    if (m_setDamageRegion) {
        EGLint rect[] = {repaint.x(), full.height() - repaint.y() - repaint.height(),
                         repaint.width(), repaint.height()};
        if (!m_setDamageRegion(m_display, eglSurface(), rect, 1)) {
            repaint = full;
            m_frameDamage = full;
        }
    }
    const double ratio = full.isEmpty()
            ? 0
            : double(repaint.width()) * repaint.height() / (double(full.width()) * full.height());
    LM_DEBUG(lingmoRender) << "repaint" << context.outputId << (repaint == full ? "full" : "partial")
                      << "damage-ratio" << ratio << "buffer-age" << age;
    return repaint;
}

bool EglRenderTarget::swap(EglSwapState &state)
{
    glFlush();
    bool ok = false;
    if (m_damagePrepared && m_swapBuffersWithDamage && state.damageEnabled()) {
        EGLint rect[] = {m_frameDamage.x(),
                         size().height() - m_frameDamage.y() - m_frameDamage.height(),
                         m_frameDamage.width(), m_frameDamage.height()};
        ok = m_swapBuffersWithDamage(m_display, eglSurface(), rect, 1);
        if (!ok) {
            state.damageFailed();
            ok = eglSwapBuffers(m_display, eglSurface());
        }
    } else {
        ok = eglSwapBuffers(m_display, eglSurface());
    }
    if (ok) {
        m_damageHistory.presented(m_damagePrepared ? m_frameDamage : base::Rect(base::Point(), size()));
        m_damageValid = true;
    } else {
        m_damageHistory.reset();
        m_damageValid = false;
    }
    if (!ok)
        LM_WARNING(lingmoRender) << "eglSwapBuffers failed";
    return ok;
}

void EglRenderTarget::invalidate()
{
    m_damageHistory.reset();
    m_damageValid = false;
}
} // namespace Lingmo::Render
