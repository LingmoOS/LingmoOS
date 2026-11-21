// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "render/egl_context.h"

#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <algorithm>
#include <map>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

#include "core/logging.h"
#include "render/egl_render_target.h"

namespace Lingmo::Render {

class EglDisplay
{
public:
    static std::shared_ptr<EglDisplay> acquire(EGLDisplay display)
    {
        if (display == EGL_NO_DISPLAY)
            return nullptr;
        std::lock_guard lock(s_mutex);
        auto &users = s_displays[display];
        if (!users && !eglInitialize(display, nullptr, nullptr)) {
            s_displays.erase(display);
            return nullptr;
        }
        ++users;
        return std::shared_ptr<EglDisplay>(new EglDisplay(display));
    }

    ~EglDisplay()
    {
        std::lock_guard lock(s_mutex);
        if (--s_displays.at(m_display) == 0) {
            eglTerminate(m_display);
            s_displays.erase(m_display);
        }
    }

private:
    explicit EglDisplay(EGLDisplay display)
        : m_display(display)
    {
    }

    EGLDisplay m_display;
    // EGL initialization is not reference-counted, even when contexts share a display.
    static std::map<EGLDisplay, size_t> s_displays;
    static std::mutex s_mutex;
};

std::map<EGLDisplay, size_t> EglDisplay::s_displays;
std::mutex EglDisplay::s_mutex;

namespace {

std::string eglString(EGLDisplay display, EGLint name)
{
    const char *value = eglQueryString(display, name);
    return value ? value : "";
}

std::string glString(GLenum name)
{
    const auto *value = glGetString(name);
    return value ? reinterpret_cast<const char *>(value) : "";
}

} // namespace

bool hasExtension(std::string_view list, std::string_view extension)
{
    if (extension.empty())
        return false;
    for (size_t from = list.find(extension); from != std::string_view::npos;
         from = list.find(extension, from + 1)) {
        const size_t end = from + extension.size();
        if ((from == 0 || list[from - 1] == ' ') && (end == list.size() || list[end] == ' '))
            return true;
    }
    return false;
}

std::shared_ptr<EglContext> EglContext::create(gbm_device *device, uint32_t format)
{
    auto getDisplay = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
            eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!getDisplay)
        return nullptr;
    auto context = std::shared_ptr<EglContext>(new EglContext);
    if (!context->initialize(getDisplay(EGL_PLATFORM_GBM_KHR, device, nullptr), format, -1))
        return nullptr;
    return context;
}

std::shared_ptr<EglContext> EglContext::createSurfaceless(int alphaSize)
{
    auto context = std::shared_ptr<EglContext>(new EglContext);
    if (!context->initialize(
                eglGetPlatformDisplay(EGL_PLATFORM_SURFACELESS_MESA, EGL_DEFAULT_DISPLAY, nullptr),
                0, alphaSize)
        || !context->makeCurrent())
        return nullptr;
    return context;
}

bool EglContext::initialize(EGLDisplay display, uint32_t format, int alphaSize)
{
    m_displayOwner = EglDisplay::acquire(display);
    if (!m_displayOwner)
        return false;
    m_display = display;
    if (!eglBindAPI(EGL_OPENGL_ES_API))
        return false;
    const EGLint attributes[] = {EGL_SURFACE_TYPE,
                                 alphaSize < 0 ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT,
                                 EGL_RENDERABLE_TYPE,
                                 EGL_OPENGL_ES2_BIT,
                                 EGL_RED_SIZE,
                                 8,
                                 EGL_GREEN_SIZE,
                                 8,
                                 EGL_BLUE_SIZE,
                                 8,
                                 EGL_ALPHA_SIZE,
                                 std::max(0, alphaSize),
                                 EGL_NONE};
    EGLint count = 0;
    if (!eglChooseConfig(m_display, attributes, nullptr, 0, &count) || !count)
        return false;
    std::vector<EGLConfig> configs(count);
    if (!eglChooseConfig(m_display, attributes, configs.data(), count, &count))
        return false;
    for (int i = 0; i < count; ++i) {
        EGLint value = 0;
        if (eglGetConfigAttrib(m_display, configs[i],
                               alphaSize < 0 ? EGL_NATIVE_VISUAL_ID : EGL_ALPHA_SIZE, &value)
            && (alphaSize < 0 ? uint32_t(value) == format : value == alphaSize)) {
            m_config = configs[i];
            break;
        }
    }
    if (!m_config)
        return false;
    const EGLint contextAttributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, contextAttributes);
    if (m_context == EGL_NO_CONTEXT)
        return false;
    m_extensions = eglString(m_display, EGL_EXTENSIONS);
    m_eglVendor = eglString(m_display, EGL_VENDOR);
    m_eglVersion = eglString(m_display, EGL_VERSION);
    LM_INFO(lingmoRender).printf("EGL vendor: %s", m_eglVendor.c_str());
    LM_INFO(lingmoRender).printf("EGL version: %s", m_eglVersion.c_str());
    return true;
}

EglContext::~EglContext()
{
    if (m_display == EGL_NO_DISPLAY)
        return;
    if (m_context != EGL_NO_CONTEXT) {
        if (isCurrent())
            eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(m_display, m_context);
    }
    m_retirementTarget.reset();
}

bool EglContext::makeCurrent(EGLSurface surface)
{
    if (surface == EGL_NO_SURFACE && !supports("EGL_KHR_surfaceless_context")) {
        if (!m_retirementTarget)
            m_retirementTarget = EglRenderTarget::createPbuffer(m_display, m_config, {1, 1});
        if (!m_retirementTarget)
            return false;
        surface = m_retirementTarget->eglSurface();
    }
    if (!eglMakeCurrent(m_display, surface, surface, m_context))
        return false;
    if (m_glesRenderer.empty()) {
        m_glExtensions = glString(GL_EXTENSIONS);
        m_glesRenderer = glString(GL_RENDERER);
        m_glesVersion = glString(GL_VERSION);
        LM_INFO(lingmoRender).printf("GLES renderer: %s", m_glesRenderer.c_str());
        LM_INFO(lingmoRender).printf("GLES version: %s", m_glesVersion.c_str());
    }
    return true;
}

bool EglContext::isCurrent() const
{
    return m_context != EGL_NO_CONTEXT && eglGetCurrentContext() == m_context;
}

bool EglContext::supports(const char *extension) const
{
    return hasExtension(m_extensions, extension);
}

bool EglContext::supportsGl(const char *extension) const
{
    return hasExtension(m_glExtensions, extension);
}

ScopedEglCurrent::ScopedEglCurrent(EglContext &context)
    : m_display(eglGetCurrentDisplay())
    , m_context(eglGetCurrentContext())
    , m_draw(eglGetCurrentSurface(EGL_DRAW))
    , m_read(eglGetCurrentSurface(EGL_READ))
    , m_owner(context)
    , m_changed(!context.isCurrent())
    , m_valid(!m_changed || context.makeCurrent())
{
}

ScopedEglCurrent::~ScopedEglCurrent()
{
    if (!m_changed || !m_valid)
        return;
    if (m_context != EGL_NO_CONTEXT)
        eglMakeCurrent(m_display, m_draw, m_read, m_context);
    else
        eglMakeCurrent(m_owner.display(), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}

} // namespace Lingmo::Render
