// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TOOLKIT_TOOLKIT_HOST_H_
#define LINGMO_TOOLKIT_TOOLKIT_HOST_H_

#include <cstdint>
#include <memory>

#include "base/region.h"
#include "ui/bitmap.h"

namespace Lingmo::Render {
class Renderer;
class Texture;
} // namespace Lingmo::Render

namespace Lingmo::ui {
class Application;
class EmbeddedWindow;
} // namespace Lingmo::UI

namespace Lingmo::Toolkit {

// The toolkit's application for the panels the compositor draws itself: one
// per process, on the embedded platform and the main loop. Created on first
// use and kept for the process's life.
ui::Application &application();

// Whether panels may draw on the GPU at all; on unless a test compares the
// CPU path.
void setGpuRenderingAllowed(bool allowed);

// Frees what Skia keeps on the GPU for later frames (scratch textures,
// cached masks and paths) that no window holds now.
void purgeGpuResources();

// Destroys a window that may have rendered on the GPU, with the context its
// GL objects live on current.
void destroyWindow(std::unique_ptr<ui::EmbeddedWindow> &window);

// Brackets the renders of one frame's windows, so that what the toolkit does
// once per frame on the GPU is done once for all of them.
class PanelBatch
{
public:
    PanelBatch();
    ~PanelBatch();
    PanelBatch(const PanelBatch &) = delete;
    PanelBatch &operator=(const PanelBatch &) = delete;
};

// What a scene node draws a toolkit window with. On a GL renderer Skia draws
// the window straight into a texture of the renderer's context; elsewhere,
// or when that fails (no context, a lost one), the window is rendered on the
// CPU and only what changed is uploaded.
class PanelTexture
{
public:
    explicit PanelTexture(ui::EmbeddedWindow *window);
    ~PanelTexture();

    // The frame time later renders step animations to, in nanoseconds on
    // CLOCK_MONOTONIC; 0 for now.
    void setFrameTime(uint64_t frameTimeNs) { m_frameTimeNs = frameTimeNs; }

    // Renders on the CPU into image() when anything changed since. Returns
    // whether the pixels changed.
    bool renderImage();
    // The CPU frame, for tests and for an output that is not drawn on the GPU.
    const ui::Bitmap &image() const { return m_image; }

    // In a node's paint: the window as a texture of `renderer`, rendered
    // first when `dirty` or when the texture is not up to date; null when
    // there is nothing to draw.
    render::Texture *texture(render::Renderer &renderer, bool dirty);
    // The same texture, for a surface node to hold; the next render that
    // needs another size or path replaces it rather than freeing it under the
    // node.
    std::shared_ptr<render::Texture> sharedTexture(render::Renderer &renderer, bool dirty);
    // What the last render through texture() changed, in device pixels; the
    // whole texture after a new one or a switch between GPU and CPU.
    const base::Region &lastDamage() const { return m_lastDamage; }
    // Frees the texture and, once, every GL object the toolkit holds.
    void releaseRenderResources();
    // Whether the last texture() drew on the GPU.
    bool lastFrameOnGpu() const { return m_lastOnGpu; }

private:
    std::shared_ptr<render::Texture> gpuTexture(render::Renderer &renderer, bool dirty);
    std::shared_ptr<render::Texture> cpuTexture(render::Renderer &renderer, bool dirty);

    ui::EmbeddedWindow *m_window;
    uint64_t m_frameTimeNs = 0;
    ui::Bitmap m_image;
    // The window scale image() was drawn at.
    double m_imageScale = 0;
    // Changed since the CPU texture was last uploaded, in image pixels.
    base::Region m_pendingUpload;
    std::shared_ptr<render::Texture> m_cpuTexture;
    std::shared_ptr<render::Texture> m_gpuTexture;
    base::Region m_lastDamage;
    // The GPU texture holds the window's latest frame; a CPU render since
    // leaves it behind.
    bool m_gpuCurrent = false;
    // A GPU frame since image() was drawn: the next CPU frame is copied whole.
    bool m_imageBehind = false;
    bool m_lastOnGpu = false;
    // A renderer whose textures are not GL ones: never asked again.
    const render::Renderer *m_cpuRenderer = nullptr;
};

} // namespace Lingmo::Toolkit

#endif // LINGMO_TOOLKIT_TOOLKIT_HOST_H_
