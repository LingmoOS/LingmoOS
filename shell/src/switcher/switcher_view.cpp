// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "switcher/switcher_view.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "base/settings.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "core/preferences.h"
#include "decoration/decoration_theme.h"
#include "decoration/shadow.h"
#include "decoration/shadow_node.h"
#include "display/display_manager.h"
#include "render/glass.h"
#include "render/renderer.h"
#include "scene/node.h"
#include "scene/window_visual_node.h"
#include "toolkit/compositor_colors.h"
#include "toolkit/toolkit_host.h"
#include "ui/embedded_window.h"
#include "ui/icon_image.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "wm/window.h"
#include "wm/window_manager.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Switcher {

namespace {

void setApplicationIconTheme(const std::string &name)
{
    ui::Theme theme = ui::Theme::current();
    theme.iconTheme = name;
    ui::Theme::setCurrent(theme);
}

void preloadApplicationIcon(const std::string &name, int pixels)
{
    if (!ui::IconImage::preload(name, pixels))
        ui::IconImage::preload(kFallbackIcon, pixels);
}

// Frosted enough that a busy wallpaper under the icons and the name calms to
// soft patches, and tinted enough to stand off a flat colour of any shade;
// the rim still bends what is behind it.
render::GlassParams panelGlass(bool darkMode)
{
    render::GlassParams glass;
    glass.enabled = true;
    // The glass blur kernel is 30 device pixels wide; a larger sigma truncates.
    glass.blurSigma = 8.0f;
    glass.displacement = 18.0f;
    glass.chroma = 0.02f;
    glass.tintRed = darkMode ? 0.14f : 1.0f;
    glass.tintGreen = darkMode ? 0.14f : 1.0f;
    glass.tintBlue = darkMode ? 0.16f : 1.0f;
    glass.tintAlpha = darkMode ? 0.62f : 0.58f;
    return glass;
}

// The selection: a smaller, frosted piece of glass that darkens what it
// covers in the light and lifts it in the dark.
render::GlassParams selectionGlass(bool darkMode)
{
    render::GlassParams glass;
    glass.enabled = true;
    glass.blurSigma = 6.0f;
    glass.displacement = 12.0f;
    glass.chroma = 0.02f;
    glass.tintRed = glass.tintGreen = glass.tintBlue = darkMode ? 1.0f : 0.0f;
    glass.tintAlpha = darkMode ? 0.14f : 0.12f;
    return glass;
}

// The logical step whose multiples land on whole device pixels at `scale`.
int pixelStep(double scale)
{
    const int numerator = base::roundToInt(scale * 120);
    return numerator > 0 ? 120 / std::gcd(numerator, 120) : 1;
}

int snapDown(double value, int step)
{
    return int(std::floor(value / step)) * step;
}

int snapUp(double value, int step)
{
    return int(std::ceil(value / step)) * step;
}

class PanelNode final : public scene::Node
{
public:
    explicit PanelNode(SwitcherView *view)
        : Node("switcher-panel")
        , m_view(view)
    {
    }

    Content content() const override { return Content::Translucent; }

    // The outline is at most one physical pixel wide, so one logical pixel covers it.
    base::Rect absolutePaintBounds() const override
    {
        return absoluteGeometry().adjusted(-1, -1, 1, 1);
    }

    void releaseRenderResources() override { m_view->releaseRenderResources(); }

protected:
    void paint(render::Renderer &renderer, const base::Rect &bounds) const override
    {
        render::Texture *texture = m_view->panelTexture(renderer);
        if (!texture)
            return;
        const decoration::Theme *theme = m_view->theme();
        const bool darkMode = theme && theme->darkMode();
        renderer.drawBackgroundBlur(bounds, SwitcherView::kRadius, bounds, panelGlass(darkMode));
        if (const base::Rect selection = m_view->selectionBounds(); !selection.isEmpty()) {
            renderer.drawBackgroundBlur(selection, m_view->selectionRadius(), bounds,
                                        selectionGlass(darkMode));
        }
        renderer.drawTextureMapped(texture, bounds, 1.0f, base::AffineTransform());
        if (!theme)
            return;
        // The same edge the shell's glass surfaces get.
        const decoration::WindowVisualStyle style = theme->visualStyle();
        scene::WindowVisualState state;
        state.shape = {base::RectF(bounds), double(SwitcherView::kRadius)};
        state.drawOutline = true;
        state.outlineWidthPx = style.outlineWidthPx;
        state.outlineColor = style.activeOutline;
        state.highlightColor = style.highlight;
        scene::paintWindowOutline(renderer, state.shape, state);
    }

private:
    SwitcherView *m_view;
};

} // namespace

// An application's icon, made at the size a full cell shows it and scaled
// smoothly into the cell it is in.
class ApplicationIcon final : public ui::IconImage
{
public:
    ApplicationIcon() { setFallbackName(kFallbackIcon); }

    void setIcon(const std::string &name, int pixels)
    {
        setName(name);
        setPixels(pixels);
    }
};

// The row of applications being switched through, most recently used first,
// with the selected one's name under its icon. The compositor draws the
// glass, the selection's included, and the edge; this is only what goes on
// them.
class SwitcherContent final : public ui::View
{
public:
    SwitcherContent()
    {
        m_name = addChild<ui::Label>();
        m_name->setFontSize(11);
        m_name->setWeight(600);
        m_name->setElide(ui::Label::Elide::Right);
    }

    void setEntries(const std::vector<ApplicationInfo> &entries, int iconPixels)
    {
        // A count, not the list: switching between as many applications again
        // keeps every cell and only changes what it shows.
        while (m_icons.size() > entries.size()) {
            takeChild(m_icons.back());
            m_icons.pop_back();
        }
        while (m_icons.size() < entries.size())
            m_icons.push_back(addChild<ApplicationIcon>());
        raiseChild(m_name);
        m_names.clear();
        for (size_t i = 0; i < entries.size(); ++i) {
            m_icons[i]->setIcon(entries[i].icon, iconPixels);
            m_names.push_back(entries[i].name);
        }
        updateName();
        requestLayout();
    }

    void setSelected(int index)
    {
        if (index == m_selected)
            return;
        m_selected = index;
        updateName();
        requestLayout();
    }

    void setMetrics(double rowX, double cellSize, double iconSize, double padding)
    {
        m_rowX = rowX;
        m_cellSize = cellSize;
        m_iconSize = iconSize;
        m_padding = padding;
        requestLayout();
    }

    void setDarkMode(bool dark)
    {
        m_name->setColor(ui::ColorProvider::forMode(dark).get(toolkit::CompositorColor::SwitcherName));
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        // Centred, whatever snapping did to the panel's height.
        const double rowY = (geometry().height - m_cellSize) / 2;
        for (size_t i = 0; i < m_icons.size(); ++i) {
            m_icons[i]->setGeometry({m_rowX + double(i) * m_cellSize + (m_cellSize - m_iconSize) / 2,
                                     rowY + (m_cellSize - m_iconSize) / 2, m_iconSize, m_iconSize});
        }
        if (!m_name->isVisible())
            return;
        // Centred under its icon, kept inside the panel at the ends of the row.
        const ui::SizeF hint = m_name->sizeHint();
        const double nameWidth = std::min(hint.width, width - 2 * m_padding);
        const double centre = m_rowX + (m_selected + 0.5) * m_cellSize;
        const double x = std::max(m_padding,
                                 std::min(centre - nameWidth / 2, width - m_padding - nameWidth));
        // Its middle a little under half way down from the selection to the
        // panel's edge, scaled with the cells.
        const double y = rowY + (m_cellSize + m_iconSize) / 2 + m_cellSize * 10.5 / 108
                - hint.height / 2;
        m_name->setGeometry({x, y, nameWidth, hint.height});
    }

private:
    void updateName()
    {
        const bool shown = m_selected >= 0 && m_selected < int(m_names.size());
        // Only the selected application's name is shown, so only it is laid out.
        m_name->setText(shown ? m_names[size_t(m_selected)] : std::string());
        m_name->setVisible(shown);
    }

    std::vector<ApplicationIcon *> m_icons;
    std::vector<std::string> m_names;
    ui::Label *m_name = nullptr;
    int m_selected = -1;
    double m_rowX = 0;
    double m_cellSize = SwitcherView::kCellSize;
    double m_iconSize = SwitcherView::kCellSize * SwitcherView::kIconRatio;
    double m_padding = SwitcherView::kPadding;
};

SwitcherView::SwitcherView(scene::Node *container, wm::WindowManager *manager,
                           const decoration::Theme *theme)
    : m_manager(manager)
    , m_theme(theme)
    , m_warmTimer(core::mainLoop())
    , m_preloadTimer(core::mainLoop())
{
    if (m_theme) {
        m_shadow = static_cast<decoration::ShadowNode *>(
                container->addChild(std::make_unique<decoration::ShadowNode>(m_theme)));
        m_shadow->setVisible(false);
    }
    m_panel = container->addChild(std::make_unique<PanelNode>(this));
    m_panel->setVisible(false);

    m_warmTimer.setSingleShot(true);
    m_warmTimer.onTimeout = [this] { warmNext(); };
    m_preloadTimer.setSingleShot(true);
    m_preloadTimer.onTimeout = [this] { preload(); };
}

void SwitcherView::preloadAfter(std::chrono::milliseconds delay)
{
    m_preloadTimer.start(delay);
}

SwitcherView::~SwitcherView()
{
    // The panel node calls back into this view; the scene outlives it.
    m_panel->parent()->removeChild(m_panel);
    if (m_shadow)
        m_shadow->parent()->removeChild(m_shadow);
    m_panelTexture.reset();
    toolkit::destroyWindow(m_window);
}

void SwitcherView::ensureWindow()
{
    if (m_window)
        return;
    toolkit::application();
    m_window = std::make_unique<ui::EmbeddedWindow>();
    auto content = std::make_unique<SwitcherContent>();
    m_content = content.get();
    m_window->setContentView(std::move(content));
    m_window->onRenderRequested = [this] { markDirty(); };
    m_panelTexture = std::make_unique<toolkit::PanelTexture>(m_window.get());
}

void SwitcherView::markDirty()
{
    m_dirty = true;
    if (m_shown)
        m_panel->notifyChanged();
}

void SwitcherView::preload()
{
    ensureWindow();
    // Drawing a name and an icon once loads the fonts and the icon theme, which
    // cost far more than the views on the first switch.
    const auto [area, scale] = outputFor({});
    layout({{"Aa", kFallbackIcon}}, 0, area, scale);
    m_panelTexture->renderImage();

    for (wm::Window *window : m_manager->windows())
        m_warmQueue.push_back(window->id());
    m_windowAdded = m_manager->windowAdded.connect([this](wm::Window *window) {
        m_warmQueue.push_back(window->id());
        if (!m_warmTimer.isActive())
            m_warmTimer.start(std::chrono::milliseconds(kWarmDelayMs));
    });
    if (!m_warmQueue.empty())
        m_warmTimer.start(std::chrono::nanoseconds(0));
}

int SwitcherView::iconPixels(double scale)
{
    return base::roundToInt(kCellSize * kIconRatio * scale);
}

void SwitcherView::followIconTheme()
{
    const base::Settings settings(preferencesFile("services"));
    const std::string theme = settings.stringValue("Appearance/IconTheme", "Lingmo");
    if (theme == m_iconTheme)
        return;
    m_iconTheme = theme;
    setApplicationIconTheme(theme);
}

void SwitcherView::warmNext()
{
    if (m_warmQueue.empty())
        return;
    const uint64_t id = m_warmQueue.front();
    m_warmQueue.pop_front();
    wm::Window *window = m_manager->windowById(id);
    if (window && !window->skipTaskbar()) {
        m_applications.refresh();
        const ApplicationInfo info =
                m_applications.lookup(window->applicationId(), window->title());
        preloadApplicationIcon(info.icon, iconPixels(outputFor({}).second));
    }
    // One window per turn of the event loop, so no frame waits for many.
    if (!m_warmQueue.empty())
        m_warmTimer.start(std::chrono::nanoseconds(0));
}

std::pair<base::Rect, double> SwitcherView::outputFor(const std::string &output) const
{
    const base::Rect area = m_manager->outputForNewWindow();
    if (const display::DisplayManager *displays = m_manager->displayManager();
        displays && displays->applied()) {
        for (const auto &logical : displays->applied()->logicalDisplays) {
            if (logical.waylandName == output || (output.empty() && logical.bounds == area))
                return {logical.bounds, logical.scale.value()};
        }
    }
    return {area, 1};
}

void SwitcherView::showWindowCycle(const std::vector<wm::Window *> &entries, int selected,
                                   const std::string &output)
{
    ensureWindow();
    m_applications.refresh();
    std::vector<ApplicationInfo> list;
    for (wm::Window *window : entries)
        list.push_back(m_applications.lookup(window->applicationId(), window->title()));
    const auto [area, scale] = outputFor(output);
    layout(list, selected, area, scale);

    m_shown = true;
    m_panel->setGeometry(m_bounds);
    m_panel->setVisible(true);
    if (m_shadow) {
        // Lifted like a window: on a flat colour as light as the glass, the
        // shadow is all that draws the panel's edge. Its corner has to be the
        // glass's, or the shadow's tighter one shows past it.
        static_assert(kRadius <= decoration::ShadowGeometry::kMaxRadius);
        m_shadow->update(m_bounds, wm::ShadowElevation::Active, decoration::Corners(), kRadius);
        m_shadow->setVisible(true);
    }
    m_panel->notifyChanged();
}

void SwitcherView::layout(const std::vector<ApplicationInfo> &entries, int selected,
                          const base::Rect &area, double scale)
{
    // Following the session's icon theme, which may have changed since.
    followIconTheme();

    m_count = static_cast<int>(entries.size());
    const int step = pixelStep(scale);
    const double available = area.width() - 2 * kScreenMargin - 2 * kPadding;
    m_cellSize = std::min(kCellSize, std::floor(available / std::max(1, m_count)));
    const double row = m_count * m_cellSize;
    const base::Size size(snapUp(row + 2 * kPadding, step),
                          snapUp(m_cellSize + 2 * kVerticalPadding, step));
    m_rowX = (size.width() - row) / 2;
    const base::Point position(snapDown(area.x() + (area.width() - size.width()) / 2.0, step),
                               snapDown(area.y() + (area.height() - size.height()) / 2.0, step));
    m_bounds = base::Rect(position, size);

    m_window->setScale(scale);
    m_window->setSize({double(size.width()), double(size.height())});
    m_content->setDarkMode(m_theme && m_theme->darkMode());
    m_content->setMetrics(m_rowX, m_cellSize, m_cellSize * kIconRatio, kPadding);
    m_content->setEntries(entries, iconPixels(scale));
    m_content->setSelected(selected);
    m_selected = selected;
    m_dirty = true;
}

void SwitcherView::selectWindowCycleEntry(int index)
{
    if (m_content)
        m_content->setSelected(index);
    m_selected = index;
    // The selection's glass is the node's, not the image's.
    if (m_shown)
        m_panel->notifyChanged();
}

void SwitcherView::hideWindowCycle()
{
    m_shown = false;
    m_panel->setVisible(false);
    if (m_shadow)
        m_shadow->setVisible(false);
}

base::Rect SwitcherView::selectionBounds() const
{
    if (!m_shown || m_selected < 0 || m_selected >= m_count)
        return {};
    const double size = m_cellSize * kSelectionRatio;
    const double inset = (m_cellSize - size) / 2;
    return base::RectF(m_bounds.x() + m_rowX + m_selected * m_cellSize + inset,
                       m_bounds.y() + rowY() + inset, size, size)
            .toNearestRect();
}

bool SwitcherView::windowCycleViewContains(const base::PointF &location) const
{
    // Both edges count as inside.
    return m_shown && location.x() >= m_bounds.x() && location.x() <= m_bounds.right()
            && location.y() >= m_bounds.y() && location.y() <= m_bounds.bottom();
}

int SwitcherView::windowCycleEntryAt(const base::PointF &location) const
{
    if (!windowCycleViewContains(location))
        return -1;
    const base::PointF local =
            location - base::PointF(m_bounds.origin()) - base::PointF(m_rowX, rowY());
    if (local.x() < 0 || local.y() < 0 || local.y() >= m_cellSize)
        return -1;
    const int index = int(local.x() / m_cellSize);
    return index < m_count ? index : -1;
}

ui::Bitmap SwitcherView::renderImage()
{
    if (!m_shown || !m_window)
        return {};
    if (m_dirty || m_panelTexture->image().isNull()) {
        m_dirty = false;
        m_panelTexture->renderImage();
    }
    return m_panelTexture->image();
}

render::Texture *SwitcherView::panelTexture(render::Renderer &renderer)
{
    if (!m_shown || !m_window)
        return nullptr;
    const bool dirty = m_dirty;
    m_dirty = false;
    return m_panelTexture->texture(renderer, dirty);
}

void SwitcherView::releaseRenderResources()
{
    if (m_panelTexture)
        m_panelTexture->releaseRenderResources();
}

} // namespace Lingmo::Switcher
