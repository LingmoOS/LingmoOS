// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"

#include "ui/platform_window.h"

class GrDirectContext;

namespace Lingmo::ui {

class EmbeddedWindow;

// The platform side of a window on Application::Platform::Embedded: frames go
// into memory the host reads, and the host hears when one is wanted and which
// cursor the pointer asks for.
class EmbeddedPlatformWindow final : public PlatformWindow
{
public:
    // A popup when it has a parent.
    EmbeddedPlatformWindow(Window* window, EmbeddedPlatformWindow* parent, const PopupPlacement& placement,
                           bool grab);
    ~EmbeddedPlatformWindow() override;

    bool isHeadless() const override { return false; }
    bool isEmbedded() const override { return true; }
    void setTitle(const std::string&) override {}
    void setAppId(const std::string&) override {}
    void setMinimumSize(int, int) override {}
    void setMaximumSize(int, int) override {}
    void setMaximized(bool) override {}
    void setFullScreen(bool) override {}
    void minimize() override {}
    void setTransientParent(PlatformWindow*, bool) override {}
    void startMove() override {}
    void activate() override;
    void setBlur(const RectF& rect, double radius) override;
    void setBlurShape(const Path& path) override;
    void setTextInput(const TextInputState* state) override;
    void show(int width, int height) override;
    void setPopupPlacement(const PopupPlacement& placement, int width, int height) override;
    void hide() override;
    bool startDrag(DragSource source) override;
    void setCursorShape(CursorShape shape) override;
    void requestRender() override;
    bool present(SizeF size, double scale, bool opaque, const PaintFunction& paint) override;

    const uint8_t* pixels() const { return m_pixels.data(); }
    int width() const { return m_width; }
    int height() const { return m_height; }
    // The host takes the next request as a new one once it rendered.
    void clearRenderRequest() { m_renderRequested = false; }
    bool isRenderRequested() const { return m_renderRequested; }

    // Set around a render into a host texture on the GPU.
    struct GpuTarget
    {
        GrDirectContext* context = nullptr;
        uint32_t texture = 0;
        int width = 0;
        int height = 0;
        bool keepsLastFrame = false;
    };
    void setGpuTarget(const GpuTarget* target) { m_gpuTarget = target; }

    std::function<void()> onRenderRequested;
    std::function<void(CursorShape)> onCursorShape;

    // The EmbeddedWindow this is the platform of; null for a plain Window,
    // which is never drawn.
    EmbeddedWindow* owner = nullptr;
    // The next state change comes from the input method.
    bool inputMethodCause = false;

    bool isPopup() const { return m_popup; }
    const PopupPlacement& placement() const { return m_placement; }
    bool grabs() const { return m_grab; }
    const RectF& blurRect() const { return m_blurRect; }
    double blurRadius() const { return m_blurRadius; }
    const Path& blurShape() const { return m_blurShape; }
    void popupRepositioned();

private:
    bool m_popup = false;
    PopupPlacement m_placement;
    bool m_grab = false;
    // Told to the host, until hidden.
    bool m_popupShown = false;
    RectF m_blurRect;
    double m_blurRadius = 0;
    Path m_blurShape;
    // What the host last heard; unset while disabled.
    std::optional<TextInputState> m_textInput;
    WindowStates m_states;
    std::vector<uint8_t> m_pixels;
    int m_width = 0;
    int m_height = 0;
    // The buffer holds the last frame, so frames repaint only what changed.
    bool m_holdsFrame = false;
    bool m_renderRequested = false;
    const GpuTarget* m_gpuTarget = nullptr;
    // The texture the last frame went into, 0 after a frame in memory.
    uint32_t m_lastTexture = 0;
    // That texture as wrapped for drawing, while it holds the last frame.
    sk_sp<SkSurface> m_gpuSurface;
    bool m_gpuSurfaceOpaque = false;
};

} // namespace Lingmo::ui
