// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/geometry.h"
#include "base/timer.h"

#include "base/signal.h"
#include "switcher/application_info.h"
#include "ui/bitmap.h"
#include "wm/window_cycle_controller.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Render {
class Renderer;
class Texture;
} // namespace Lingmo::Render

namespace Lingmo::Toolkit {
class PanelTexture;
}

namespace Lingmo::ui {
class EmbeddedWindow;
}

namespace Lingmo::Decoration {
class ShadowNode;
class Theme;
} // namespace Lingmo::Decoration

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::WM {
class WindowManager;
}

namespace Lingmo::Switcher {

class SwitcherContent;

// The application switcher's list: a row of application icons on a glass
// panel, centred on the output being switched on, with the selected one's
// name under it. Drawn by the compositor itself, as the lock screen is: a
// toolkit view tree rendered into an image the panel node composites over
// the backdrop blur. Upstream: WindowCycleView.
class SwitcherView : public wm::WindowCycleView
{
public:
    // Logical pixels. Cells shrink together until the row fits the output.
    // The distance from one icon to the next: the icons' artwork, which themes
    // draw a few pixels inside the icon's square, stands a quarter of its
    // width apart. The selection hugs the artwork.
    static constexpr double kCellSize = 108;
    static constexpr double kIconRatio = 96.0 / 108;
    static constexpr double kSelectionRatio = 98.0 / 108;
    // Beside the row.
    static constexpr double kPadding = 16;
    // Above and below the row, alike, so the icons sit in the middle of the
    // panel as far from every edge; the selected one's name goes just under
    // its selection.
    static constexpr double kVerticalPadding = 18;
    static constexpr double kScreenMargin = 40;
    static constexpr int kRadius = 36;
    // At full size; it shrinks with the cells.
    static constexpr double kSelectionRadius = 22;

    SwitcherView(scene::Node *container, wm::WindowManager *manager,
                 const decoration::Theme *theme);
    ~SwitcherView() override;

    // Builds the view tree ahead of the first switch, which would otherwise
    // pay for the fonts and the icon theme while the list appears.
    // From then on the application of every window that maps is looked up and
    // its icon rendered, one window at a time while the compositor is idle.
    void preload();
    // preload() once `delay` has passed; the view going first cancels it.
    void preloadAfter(std::chrono::milliseconds delay);

    static constexpr int kWarmDelayMs = 200;

    // --- wm::WindowCycleView ---

    void showWindowCycle(const std::vector<wm::Window *> &entries, int selected,
                         const std::string &output) override;
    void selectWindowCycleEntry(int index) override;
    void hideWindowCycle() override;
    bool windowCycleViewContains(const base::PointF &location) const override;
    int windowCycleEntryAt(const base::PointF &location) const override;

    // The panel's contents, rendered anew on the CPU when they changed.
    ui::Bitmap renderImage();
    // In the panel node's paint: its contents as a texture of the renderer,
    // drawn on the GPU where the renderer allows.
    render::Texture *panelTexture(render::Renderer &renderer);
    void releaseRenderResources();
    base::Rect bounds() const { return m_bounds; }
    // The selected entry's own glass, in global logical coordinates; empty
    // with nothing selected.
    base::Rect selectionBounds() const;
    int selectionRadius() const { return base::roundToInt(kSelectionRadius * m_cellSize / kCellSize); }
    const decoration::Theme *theme() const { return m_theme; }

private:
    void ensureWindow();
    void markDirty();
    // The logical bounds and scale of the output named `output`.
    std::pair<base::Rect, double> outputFor(const std::string &output) const;
    // Sizes the image and the views for `entries` centred on `area`.
    void layout(const std::vector<ApplicationInfo> &entries, int selected, const base::Rect &area,
                double scale);
    // The row's top in the panel, centred in it.
    double rowY() const { return (m_bounds.height() - m_cellSize) / 2; }
    // The icon size cells have at full size, in device pixels: icons are made
    // once at it and scaled into shrunken cells.
    static int iconPixels(double scale);
    void followIconTheme();
    void warmNext();

    wm::WindowManager *m_manager;
    const decoration::Theme *m_theme;
    ApplicationInfoCache m_applications;
    std::string m_iconTheme;
    std::deque<uint64_t> m_warmQueue;
    base::Timer m_warmTimer;
    base::Timer m_preloadTimer;
    std::unique_ptr<ui::EmbeddedWindow> m_window;
    std::unique_ptr<toolkit::PanelTexture> m_panelTexture;
    SwitcherContent *m_content = nullptr;
    decoration::ShadowNode *m_shadow = nullptr;
    scene::Node *m_panel = nullptr;
    base::Rect m_bounds;
    double m_rowX = 0;
    double m_cellSize = kCellSize;
    int m_count = 0;
    int m_selected = -1;
    bool m_shown = false;
    bool m_dirty = false;
    base::Subscription m_windowAdded;
};

} // namespace Lingmo::Switcher

