// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <EGL/egl.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

struct gbm_device;

namespace Lingmo::Render {

class EglRenderTarget;
class EglDisplay;

// Whether the space-separated `list` names `extension` whole. Asked on every
// render pass, so without splitting the list into allocations each time.
bool hasExtension(std::string_view list, std::string_view extension);

class EglContext
{
public:
    static std::shared_ptr<EglContext> create(gbm_device *device, uint32_t format);
    static std::shared_ptr<EglContext> createSurfaceless(int alphaSize = 8);
    ~EglContext();
    EglContext(const EglContext &) = delete;
    EglContext &operator=(const EglContext &) = delete;
    bool makeCurrent(EGLSurface surface = EGL_NO_SURFACE);
    bool isCurrent() const;

    EGLDisplay display() const { return m_display; }

    EGLConfig config() const { return m_config; }

    bool supports(const char *extension) const;
    bool supportsGl(const char *extension) const;

    const std::string &renderer() const { return m_glesRenderer; }

    const std::string &glVersion() const { return m_glesVersion; }

private:
    EglContext() = default;
    bool initialize(EGLDisplay display, uint32_t format, int alphaSize);
    std::shared_ptr<EglDisplay> m_displayOwner;
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLConfig m_config = nullptr;
    EGLContext m_context = EGL_NO_CONTEXT;
    std::unique_ptr<EglRenderTarget> m_retirementTarget;
    std::string m_extensions;
    std::string m_glExtensions;
    std::string m_eglVendor;
    std::string m_eglVersion;
    std::string m_glesRenderer;
    std::string m_glesVersion;
};

class ScopedEglCurrent
{
public:
    explicit ScopedEglCurrent(EglContext &context);
    ~ScopedEglCurrent();
    ScopedEglCurrent(const ScopedEglCurrent &) = delete;
    ScopedEglCurrent &operator=(const ScopedEglCurrent &) = delete;

    bool valid() const { return m_valid; }

private:
    EGLDisplay m_display;
    EGLContext m_context;
    EGLSurface m_draw;
    EGLSurface m_read;
    EglContext &m_owner;
    bool m_changed;
    bool m_valid;
};

} // namespace Lingmo::Render
