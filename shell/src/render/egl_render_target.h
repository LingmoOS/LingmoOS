// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "render/damage.h"
#include "render/renderer.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "base/region.h"
struct gbm_surface;

namespace Lingmo::Render {

class EglSwapState
{
public:
    bool damageEnabled() const { return !m_damageFailed; }

    void damageFailed() { m_damageFailed = true; }

private:
    bool m_damageFailed = false;
};

// An EGL window surface on a gbm_surface.
class EglRenderTarget : public RenderTarget
{
public:
    EglRenderTarget(EGLDisplay display, EGLSurface surface, gbm_surface *gbmSurface,
                    const base::Size &size);
    ~EglRenderTarget() override;

    base::Size size() const override { return m_size; }

    EGLSurface eglSurface() const { return m_surface; }

    static std::unique_ptr<EglRenderTarget> create(EGLDisplay display, EGLConfig config,
                                                   gbm_surface *surface, const base::Size &size);
    static std::unique_ptr<EglRenderTarget> createPbuffer(EGLDisplay display, EGLConfig config,
                                                          const base::Size &size);

    void beginFrame() { m_damagePrepared = false; }

    bool damagePrepared() const { return m_damagePrepared; }

    base::Rect prepareDamage(const base::Region &damage, const std::vector<base::Rect> &samples,
                        const OutputRenderContext &context);
    bool swap(EglSwapState &state);
    void invalidate();

private:
    base::Rect m_frameDamage;
    bool m_damagePrepared = false;
    BufferDamageHistory m_damageHistory;
    OutputRenderContext m_damageContext;
    bool m_damageValid = false;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLSurface m_surface = EGL_NO_SURFACE;
    gbm_surface *m_gbmSurface = nullptr;
    base::Size m_size;
    // Resolved once: the extension string does not change for a display.
    bool m_bufferAge = false;
    PFNEGLSETDAMAGEREGIONKHRPROC m_setDamageRegion = nullptr;
    PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC m_swapBuffersWithDamage = nullptr;
};

} // namespace Lingmo::Render
