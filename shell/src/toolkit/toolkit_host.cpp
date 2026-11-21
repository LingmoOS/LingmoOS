// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "toolkit/toolkit_host.h"

#include <algorithm>
#include <cstring>
#include <vector>

#include "core/logging.h"
#include "render/egl_context.h"
#include "render/gl_texture.h"
#include "render/renderer.h"
#include "ui/application.h"
#include "ui/embedded_gpu.h"
#include "ui/embedded_window.h"

namespace Lingmo::Toolkit {

namespace {

// Skia on the compositor's own context, shared by every panel. The context
// is kept so its GL objects can be freed with it current.
struct GpuHost {
    std::shared_ptr<render::EglContext> context;
    std::unique_ptr<ui::EmbeddedGpu> gpu;
    // Once Skia could not start or lost its context, the panels stay on the CPU.
    bool failed = false;
    bool allowed = true;
};

GpuHost &gpuHost()
{
    static GpuHost host;
    return host;
}

void closeBatch();

// The GPU for the context current now, made the first time.
ui::EmbeddedGpu *currentGpu(const std::shared_ptr<render::EglContext> &context)
{
    GpuHost &host = gpuHost();
    if (host.failed || !host.allowed || !context)
        return nullptr;
    if (host.gpu && host.gpu->isAbandoned()) {
        LM_WARNING(lingmoAurora) << "The panels' GPU context was lost; drawing them on the CPU";
        closeBatch();
        host.gpu.reset();
        host.failed = true;
        return nullptr;
    }
    if (!host.gpu) {
        host.gpu = ui::EmbeddedGpu::createForCurrentContext();
        if (!host.gpu) {
            LM_WARNING(lingmoAurora) << "Skia cannot draw on the renderer's context; drawing the panels on the CPU";
            host.failed = true;
            return nullptr;
        }
        host.context = context;
    }
    // Another renderer's context: the CPU draws for it.
    return host.context == context ? host.gpu.get() : nullptr;
}

// The batch PanelBatch brackets: opened on the first GPU render inside it,
// and closed before any GL work of the compositor's own in between.
struct BatchState {
    int depth = 0;
    ui::EmbeddedGpu *gpu = nullptr;
    bool open = false;
};

BatchState &batchState()
{
    static BatchState state;
    return state;
}

void openBatch(ui::EmbeddedGpu *gpu)
{
    BatchState &state = batchState();
    if (state.depth == 0 || state.open)
        return;
    state.gpu = gpu;
    state.open = gpu->beginBatch();
}

void closeBatch()
{
    BatchState &state = batchState();
    if (state.open)
        state.gpu->endBatch();
    state.open = false;
    state.gpu = nullptr;
}

} // namespace

ui::Application &application()
{
    // On the thread's current loop, the main loop.
    static ui::Application *instance =
            new ui::Application(0, nullptr, ui::Application::Platform::Embedded);
    return *instance;
}

void setGpuRenderingAllowed(bool allowed)
{
    gpuHost().allowed = allowed;
}

void purgeGpuResources()
{
    GpuHost &host = gpuHost();
    if (!host.gpu || host.gpu->isAbandoned() || !host.context || batchState().open)
        return;
    render::ScopedEglCurrent current(*host.context);
    if (current.valid())
        host.gpu->purgeResources();
}

void destroyWindow(std::unique_ptr<ui::EmbeddedWindow> &window)
{
    if (!window)
        return;
    GpuHost &host = gpuHost();
    if (host.gpu && !host.gpu->isAbandoned() && host.context) {
        render::ScopedEglCurrent current(*host.context);
        window.reset();
        return;
    }
    window.reset();
}

PanelBatch::PanelBatch()
{
    ++batchState().depth;
}

PanelBatch::~PanelBatch()
{
    if (--batchState().depth == 0)
        closeBatch();
}

PanelTexture::PanelTexture(ui::EmbeddedWindow *window)
    : m_window(window)
{
}

PanelTexture::~PanelTexture() = default;

bool PanelTexture::renderImage()
{
    const ui::EmbeddedWindow::Frame &frame = m_window->render(m_frameTimeNs);
    if (!frame.pixels || frame.width <= 0 || frame.height <= 0)
        return false;
    m_gpuCurrent = false;
    const base::Size size(frame.width, frame.height);
    // The damage is since the window's last frame, which may have gone to the GPU.
    const bool whole = m_image.size() != size || m_imageScale != m_window->scale() || m_imageBehind;
    m_imageBehind = false;
    if (whole) {
        // The upload's format, so no frame converts it.
        m_image = ui::Bitmap(size, ui::Bitmap::Format::Rgba8888Premultiplied);
        m_imageScale = m_window->scale();
    } else if (frame.damage.empty()) {
        return false;
    }
    base::Region changed;
    if (whole)
        changed = base::Rect(size);
    for (const ui::Rect &rect : frame.damage)
        changed |= base::Rect(rect.x, rect.y, rect.width, rect.height) & base::Rect(size);
    for (const base::Rect &rect : changed) {
        for (int row = rect.y(); row < rect.bottom(); ++row) {
            std::memcpy(m_image.scanLine(row) + size_t(rect.x()) * 4,
                        frame.pixels + size_t(row) * frame.stride + size_t(rect.x()) * 4,
                        size_t(rect.width()) * 4);
        }
    }
    m_pendingUpload |= changed;
    return true;
}

render::Texture *PanelTexture::texture(render::Renderer &renderer, bool dirty)
{
    return sharedTexture(renderer, dirty).get();
}

std::shared_ptr<render::Texture> PanelTexture::sharedTexture(render::Renderer &renderer, bool dirty)
{
    const bool wasOnGpu = m_lastOnGpu;
    m_lastDamage.clear();
    if (std::shared_ptr<render::Texture> texture = gpuTexture(renderer, dirty)) {
        m_lastOnGpu = true;
        if (!wasOnGpu)
            m_lastDamage = base::Rect(base::Point(), texture->size());
        return texture;
    }
    m_lastOnGpu = false;
    std::shared_ptr<render::Texture> texture = cpuTexture(renderer, dirty);
    if (texture && wasOnGpu)
        m_lastDamage = base::Rect(base::Point(), texture->size());
    return texture;
}

std::shared_ptr<render::Texture> PanelTexture::gpuTexture(render::Renderer &renderer, bool dirty)
{
    if (&renderer == m_cpuRenderer || !gpuHost().allowed)
        return nullptr;
    // The frame's animations may resize the window: the texture takes the
    // size they leave.
    if (dirty || !m_gpuCurrent)
        m_window->beginFrame(m_frameTimeNs);
    const base::Size size(m_window->pixelWidth(), m_window->pixelHeight());
    if (size.isEmpty())
        return nullptr;
    auto *texture = dynamic_cast<render::GlTexture *>(m_gpuTexture.get());
    if (!texture || m_gpuTexture->size() != size) {
        m_gpuTexture.reset();
        m_gpuCurrent = false;
        // The upload is the compositor's GL work, which a batch may not hold.
        closeBatch();
        // Only to learn whether the renderer is GL, and on which context.
        const std::vector<uint8_t> clear(size_t(size.width()) * size.height() * 4, 0);
        m_gpuTexture = renderer.createTexture(size, clear.data(), size.width() * 4,
                                              render::PixelFormat::Rgba8888);
        texture = dynamic_cast<render::GlTexture *>(m_gpuTexture.get());
        if (!texture || texture->target() != 0x0DE1 /* GL_TEXTURE_2D */) {
            m_gpuTexture.reset();
            m_cpuRenderer = &renderer;
            return nullptr;
        }
    }
    ui::EmbeddedGpu *gpu = currentGpu(texture->context());
    if (!gpu)
        return nullptr;
    if (!dirty && m_gpuCurrent)
        return m_gpuTexture;
    const bool keepsLastFrame = m_gpuCurrent;
    openBatch(gpu);
    const ui::EmbeddedWindow::Frame *frame = m_window->render(
            *gpu, {texture->name(), size.width(), size.height(), keepsLastFrame}, m_frameTimeNs);
    if (!frame) {
        m_gpuCurrent = false;
        return nullptr;
    }
    if (keepsLastFrame) {
        for (const ui::Rect &rect : frame->damage)
            m_lastDamage |= base::Rect(rect.x, rect.y, rect.width, rect.height) & base::Rect(size);
    } else {
        m_lastDamage = base::Rect(size);
    }
    m_gpuCurrent = true;
    m_imageBehind = true;
    return m_gpuTexture;
}

std::shared_ptr<render::Texture> PanelTexture::cpuTexture(render::Renderer &renderer, bool dirty)
{
    closeBatch();
    if (dirty || m_image.isNull())
        renderImage();
    if (m_image.isNull())
        return nullptr;
    if (m_cpuTexture && m_cpuTexture->size() == m_image.size()) {
        if (!m_pendingUpload.isEmpty()
            && !renderer.updateTexture(m_cpuTexture.get(), m_image.constBits(),
                                       int(m_image.stride()), render::PixelFormat::Rgba8888,
                                       m_pendingUpload)) {
            m_cpuTexture.reset();
        }
    } else {
        m_cpuTexture.reset();
    }
    m_lastDamage = m_pendingUpload;
    if (!m_cpuTexture) {
        // The renderer uploads the texture's own size and cannot see the
        // image's, so a panel redrawn at another size needs a new one.
        m_cpuTexture = renderer.createTexture(m_image.size(), m_image.constBits(),
                                              int(m_image.stride()),
                                              render::PixelFormat::Rgba8888);
        m_lastDamage = m_image.rect();
    }
    m_pendingUpload.clear();
    return m_cpuTexture;
}

void PanelTexture::releaseRenderResources()
{
    m_cpuRenderer = nullptr;
    m_cpuTexture.reset();
    m_gpuTexture.reset();
    m_gpuCurrent = false;
    m_pendingUpload = m_image.rect();
    GpuHost &host = gpuHost();
    closeBatch();
    if (host.gpu && host.context) {
        {
            render::ScopedEglCurrent current(*host.context);
            if (current.valid())
                host.gpu->release();
            else
                host.gpu->abandon();
        }
        host.gpu.reset();
        host.context.reset();
    }
}

} // namespace Lingmo::Toolkit
