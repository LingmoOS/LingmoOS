// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/dock/dock_window.h"

#include <algorithm>
#include <cmath>
#include <set>

#include "base/dbus.h"
#include "base/i18n.h"
#include "base/strings.h"
#include "base/url.h"
#include "core/main_loop.h"
#include "input/drag.h"
#include "input/seat.h"
#include "shell/common/application_badges.h"
#include "shell/common/task_manager.h"
#include "shell/common/task_windows.h"
#include "shell/dock/dock.h"
#include "shell/dock/dock_bounce.h"
#include "shell/dock/dock_icon.h"
#include "shell/dock/dock_magnification.h"
#include "shell/dock/dock_presentation.h"
#include "shell/dock/dock_tooltip.h"
#include "shell/shell_data.h"
#include "shell/shell_window.h"
#include "ui/application.h"
#include "ui/bubble_shape.h"
#include "ui/drag.h"
#include "ui/embedded_window.h"
#include "ui/image.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "toolkit/compositor_colors.h"

using Lingmo::i18n::tr;

namespace Lingmo::Shell {

namespace {

// The launcher's own D-Bus service, org.lingmo.Launcher.
void callLauncher(base::dbus::Connection *bus, const char *method)
{
    if (!bus)
        return;
    bus->send(bus->newMethodCall("org.lingmo.Launcher", "/Launcher", "org.lingmo.Launcher",
                                 method));
}

// The dock's glass, by the GlassTint slider:
// 0 is More Clear, 1 More Tinted, 0.5 the default.
constexpr double kPace[] = {0.000, 0.048, 0.116, 0.174, 0.249, 0.314, 0.361,
                            0.398, 0.435, 0.469, 0.500, 0.587, 0.630, 0.666,
                            0.701, 0.739, 0.781, 0.827, 0.879, 0.931, 1.000};

double glassAmount(double tint)
{
    const int last = int(std::size(kPace)) - 1;
    const double x = std::clamp(tint, 0.0, 1.0) * last;
    const int i = std::min(int(std::floor(x)), last - 1);
    return kPace[i] + (x - i) * (kPace[i + 1] - kPace[i]);
}

// A monotone cubic through the More Clear, default and More Tinted values.
double glassBlend(double amount, double clearValue, double defaultValue, double tintedValue)
{
    const double d0 = 2.0 * (defaultValue - clearValue);
    const double d1 = 2.0 * (tintedValue - defaultValue);
    const double middle = d0 * d1 > 0 ? 2.0 * d0 * d1 / (d0 + d1) : 0.0;
    const bool upper = amount > 0.5;
    const double t = upper ? 2.0 * amount - 1.0 : 2.0 * amount;
    const double y0 = upper ? defaultValue : clearValue;
    const double y1 = upper ? tintedValue : defaultValue;
    const double m0 = 0.5 * (upper ? middle : d0);
    const double m1 = 0.5 * (upper ? d1 : middle);
    const double t2 = t * t;
    const double t3 = t2 * t;
    return (2.0 * t3 - 3.0 * t2 + 1.0) * y0 + (t3 - 2.0 * t2 + t) * m0
            + (3.0 * t2 - 2.0 * t3) * y1 + (t3 - t2) * m1;
}

// A thumbnail is fitted into its slot, and so is the window landing on it.
base::Rect fitToWindow(const base::Rect &slot, const base::Rect &window)
{
    if (slot.isEmpty() || window.isEmpty())
        return slot;
    const double scale = std::min(double(slot.width()) / window.width(),
                                  double(slot.height()) / window.height());
    const double width = window.width() * scale;
    const double height = window.height() * scale;
    const double x = slot.x() + slot.width() / 2.0 - width / 2;
    const double y = slot.y() + slot.height() / 2.0 - height / 2;
    // The smallest whole-pixel rect that contains it.
    const int left = int(std::floor(x));
    const int top = int(std::floor(y));
    const base::Rect aligned(left, top, int(std::ceil(x + width)) - left,
                             int(std::ceil(y + height)) - top);
    return aligned.intersected(slot);
}

// A rounded rect as a region, row by row, as a filled polygon covers pixels.
base::Region roundedRegion(const base::Rect &rect, double radius)
{
    base::Region region;
    const int height = rect.height();
    radius = std::min(radius, std::min(rect.width(), rect.height()) / 2.0);
    for (int y = 0; y < height; ++y) {
        const double centre = y + 0.5;
        double inset = 0;
        const double dy = centre < radius ? radius - centre
                : centre > height - radius ? centre - (height - radius)
                                           : 0;
        if (dy > 0)
            inset = radius - std::sqrt(std::max(0.0, radius * radius - dy * dy));
        const int left = int(std::lround(inset));
        if (rect.width() - 2 * left > 0)
            region |= base::Rect(rect.x() + left, rect.y() + y, rect.width() - 2 * left, 1);
    }
    return region;
}

} // namespace

// The window's content: the dividers, drawn here, and the icons as children.
class DockWindow::Panel final : public ui::View
{
public:
    explicit Panel(DockWindow *dock)
        : m_dock(dock)
    {
        setTracksHover(true);
    }

protected:
    void hoverChanged() override
    {
        // Leaving the dock itself takes the tooltip down at once, and lets
        // the magnified icons settle back.
        if (!isHovered()) {
            m_dock->hideTooltip();
            m_dock->panelHoverLeft();
        }
    }

    // The pointer's place on the dock, for the magnification; the chain of
    // hovered views includes this panel while the pointer is on any of its
    // children too.
    void hoverMove(const ui::PointerEvent &event) override { m_dock->panelHoverMoved(event.position); }

    // A theme change lays every window out again.
    void layout() override
    {
        if (m_dock->m_glassDark != ui::Theme::current().darkMode)
            m_dock->updateGlass();
    }

    void paint(ui::Painter &painter) override
    {
        if (m_dock->dock().settings().efficientMode())
            painter.fillRect(localRect(), ui::Theme::current().background);
        const ui::Color line = ui::color(toolkit::CompositorColor::DockDivider);
        const base::Rect panel = m_dock->panelRect();
        const auto draw = [&](const base::RectF &rect, double opacity) {
            if (rect.isEmpty() || opacity <= 0)
                return;
            ui::Color color = line;
            color.a = uint8_t(std::lround(255 * opacity));
            painter.drawRoundedRect({panel.x() + rect.x(), panel.y() + rect.y(), rect.width(),
                                     rect.height()},
                                    {0, 0, ui::Corners::Circular}, color);
        };
        if (!m_dock->dock().settings().efficientMode()) {
            draw(m_dock->divider(), 0.18);
            draw(m_dock->trailingDivider(), m_dock->m_trailingOpacity);
        }
    }

    // The whole window takes drags: the application area, and the icons
    // that react to one hovering them.
    bool canDrop(const ui::DropOffer &) const override { return true; }

    void dragEnter(const ui::DragEvent &event) override
    {
        m_dock->dropEnter(event.offer, event.position);
    }

    uint32_t dragUpdate(const ui::DragEvent &event) override
    {
        return m_dock->dropMove(event.position);
    }

    void dragLeave() override { m_dock->dropLeave(); }

    uint32_t drop(const ui::DragEvent &event) override { return m_dock->dropAt(event.position); }

private:
    DockWindow *m_dock;
};

DockWindow::DockWindow(Shell *shell, std::unique_ptr<Dock> dock)
    : m_shell(shell)
    , m_dock(std::move(dock))
    , m_raiseTimer(core::mainLoop())
    , m_reorderTimer(core::mainLoop())
    , m_tooltipHide(core::mainLoop())
    , m_strutSettle(core::mainLoop())
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Dock);
    m_window->setFocusPolicy(wm::FocusPolicy::NoFocus);
    auto panel = std::make_unique<Panel>(this);
    m_panel = panel.get();
    m_window->setContentView(std::move(panel));
    m_window->onFrameProduced = [this] {
        handOffLanded();
        if (std::exchange(m_releaseReturn, false))
            m_shell->seat()->drag()->releaseReturn();
    };

    ui::Window *window = m_window->window();
    m_applications = std::make_unique<DockPresentation>(window);
    m_minimized = std::make_unique<DockPresentation>(window);
    // Not for minimized windows, which fly into their slots themselves.
    m_applications->setOpensForNewIcons(true);
    m_applications->onChanged = [this] { layoutItems(); };
    m_minimized->onChanged = [this] { layoutItems(); };

    m_magnification = std::make_unique<DockMagnification>(window);
    m_bounce = std::make_unique<DockBounce>(window);
    m_attentionBounce = std::make_unique<DockBounce>(window);
    m_magnification->onChanged = [this] { layoutItems(); };
    m_bounce->onChanged = [this] { layoutItems(); };
    m_attentionBounce->onChanged = [this] { layoutItems(); };
    configureMagnification();

    m_tooltip = std::make_unique<DockTooltip>(this, shell);
    m_trash = m_panel->addChild<DockIcon>(this, DockIcon::Kind::Trash);
    m_trash->setToolTip(tr("Trash"));

    m_tooltipHide.setSingleShot(true);
    m_tooltipHide.setInterval(std::chrono::milliseconds(150));
    m_tooltipHide.onTimeout = [this] { hideTooltip(); };
    m_strutSettle.setSingleShot(true);
    m_strutSettle.setInterval(std::chrono::milliseconds(200));
    m_strutSettle.onTimeout = [this] { updateStrut(); };
    m_raiseTimer.setSingleShot(true);
    m_raiseTimer.setInterval(std::chrono::milliseconds(300));
    m_raiseTimer.onTimeout = [this] {
        if (m_drop.hovered && m_drop.hovered->kind() == DockIcon::Kind::Application)
            m_dock->applications().raiseWindow(m_drop.hovered->key());
    };
    m_reorderTimer.setSingleShot(true);
    m_reorderTimer.setInterval(std::chrono::milliseconds(90));
    m_reorderTimer.onTimeout = [this] {
        // Wait out the previous swap's animation, or the sliding thumbnails
        // each claim the drag and it bounces.
        if (m_minimized->isRunning()) {
            m_reorderTimer.start();
            return;
        }
        reorderMinimized();
    };

    input::DragController *drag = m_shell->seat()->drag();
    m_subscriptions.push_back(drag->started.connect([this] { onDragStarted(); }));
    m_subscriptions.push_back(drag->locationChanged.connect([this](const base::PointF &location) {
        onDragMoved(location);
    }));
    m_subscriptions.push_back(drag->released.connect([this] {
        if (m_drag && m_drag->started)
            m_drag->removeOnDrop = m_drag->removes;
    }));
    m_subscriptions.push_back(drag->ended.connect([this] { onDragEnded(); }));
    m_subscriptions.push_back(drag->returned.connect([this] {
        if (!m_drag || !m_drag->returning)
            return;
        // Revealed whole in the open slot, where the compositor's copy waits.
        finishIconDrag();
        m_releaseReturn = true;
        m_panel->requestPaint();
    }));

    DockModel &applications = m_dock->applications();
    base::dbus::Connection *bus = m_shell->bus();
    applications.hideLauncher = [bus] { callLauncher(bus, "hide"); };
    applications.toggleLauncher = [bus] { callLauncher(bus, "toggle"); };
    m_dock->minimizedWindows().hideLauncher = [bus] { callLauncher(bus, "hide"); };
    m_dock->primaryGeometry = [this] { return primaryGeometry(); };

    const auto changed = [this] {
        rebuild();
        resize(true);
    };
    for (ListModelSignals *model : std::initializer_list<ListModelSignals *> {
                 &applications, &m_dock->minimizedWindows()}) {
        m_subscriptions.push_back(model->rowsInserted.connect([changed](int, int) { changed(); }));
        m_subscriptions.push_back(model->rowsRemoved.connect([changed](int, int) { changed(); }));
        m_subscriptions.push_back(model->rowMoved.connect([changed](int, int) { changed(); }));
        m_subscriptions.push_back(model->dataChanged.connect([this](int, int) { rebuild(); }));
        m_subscriptions.push_back(model->modelReset.connect(changed));
    }
    m_subscriptions.push_back(applications.sectionsChanged.connect(changed));
    m_subscriptions.push_back(m_shell->badges().changed.connect([this] {
        for (const auto &[id, icon] : m_applicationIcons)
            icon->setCount(m_shell->badges().count(id));
    }));
    // What is there when the dock first appears is shown at once; later changes animate.
    m_subscriptions.push_back(applications.loaded.connect([this] {
        rebuild();
        resize(false);
        m_applications->setAnimationsEnabled(true);
        m_minimized->setAnimationsEnabled(true);
    }));
    DockSettings &settings = m_dock->settings();
    m_subscriptions.push_back(settings.efficientModeChanged.connect([this] {
        hideTooltip();
        if (m_shell->seat()->drag()->busy()) m_shell->seat()->drag()->cancel();
        if (m_menu) m_menu->close();
        m_bounce->clear(); m_attentionBounce->clear();
        configureMagnification(); resize(false);
    }));
    m_subscriptions.push_back(settings.ready.connect([this] { resize(false); }));
    m_subscriptions.push_back(settings.iconSizeChanged.connect([this] {
        resize(false);
        m_strutSettle.start();
    }));
    m_subscriptions.push_back(settings.directionChanged.connect([this] {
        hideTooltip();
        resize(false);
    }));
    m_subscriptions.push_back(settings.visibilityChanged.connect([this] { resize(false); }));
    m_subscriptions.push_back(settings.glassTintChanged.connect([this] { updateGlass(); }));
    m_subscriptions.push_back(settings.minimizedAreaChanged.connect([this] {
        m_sentTargets.clear();
        updateMinimizeTargets();
    }));
    m_subscriptions.push_back(settings.magnificationChanged.connect([this] {
        configureMagnification();
        resize(false);
    }));
    m_subscriptions.push_back(settings.magnificationFactorChanged.connect([this] {
        configureMagnification();
        resize(false);
    }));
    m_subscriptions.push_back(settings.bounceOnLaunchChanged.connect([this] {
        if (!m_dock->settings().bounceOnLaunch())
            m_bounce->clear();
    }));
    m_subscriptions.push_back(m_dock->trash().emptyChanged.connect([this] { rebuild(); }));
    m_subscriptions.push_back(ui::Application::instance()->languageChanged.connect([this] { retranslate(); }));
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] { resize(false); }));
    // A side dock is centred in the work area, which other panels' zones and
    // its own former bottom zone change after it was placed.
    m_subscriptions.push_back(m_shell->windowManager()->exclusiveZonesChanged.connect([this] {
        const int thickness = DockMetrics::forIconSize(m_dock->settings().iconSize()).thickness();
        if (windowRect(thickness) != m_windowRect)
            resize(false);
    }));

    TaskWindows &windows = m_shell->taskWindows();
    m_subscriptions.push_back(windows.windowAdded.connect([this](uint64_t) {
        updateMinimizeTargets();
    }));
    m_subscriptions.push_back(windows.windowChanged.connect([this](uint64_t id, uint32_t changes) {
        if (changes & kAttentionChange)
            attentionChanged(id);
        // The target is fitted to the window's shape and follows its state.
        if (changes & (kGeometryChange | kStateChange | kSkipTaskbarChange))
            updateMinimizeTargets();
    }));
    m_subscriptions.push_back(windows.windowRemoved.connect([this](uint64_t id) {
        m_sentTargets.erase(id);
    }));
    m_subscriptions.push_back(windows.minimizeLanded.connect([this](uint64_t id) {
        // The model reveals the thumbnail; the window manager stops drawing
        // the window once a frame with it is composed.
        if (std::find(m_landed.begin(), m_landed.end(), id) == m_landed.end())
            m_landed.push_back(id);
        m_window->window()->contentView()->requestPaint();
    }));

    rebuild();
    resize(false);
    if (applications.isLoaded()) {
        m_applications->setAnimationsEnabled(true);
        m_minimized->setAnimationsEnabled(true);
    }
    m_window->show();
}

DockWindow::~DockWindow()
{
    *m_alive = false;
    m_subscriptions.clear();
    m_retired.clear();
    m_dock->primaryGeometry = nullptr;
    m_menu.reset();
    m_tooltip.reset();
    m_bounce.reset();
    m_attentionBounce.reset();
    m_magnification.reset();
    m_applications.reset();
    m_minimized.reset();
    m_window.reset();
    m_dock.reset();
}

void DockWindow::setTaskbarRightWidth(int width)
{
    width = std::max(0, width);
    if (width == m_taskbarRightWidth) return;
    m_taskbarRightWidth = width;
    if (m_dock->settings().efficientMode()) resize(false);
}

double DockWindow::pixelRatio() const
{
    return m_window->scale();
}

base::Rect DockWindow::primaryGeometry() const
{
    return m_panelRect.translated(m_windowRect.topLeft());
}

double DockWindow::cornerRadius() const
{
    if (m_dock->settings().efficientMode()) return 0;
    // From the panel as drawn, which is between two sizes while it resizes;
    // magnification keeps the bar's base thickness, so its corners come from
    // that thickness.
    const bool bottom = m_dock->settings().direction() == DockSettings::Bottom;
    const double drawn = bottom ? m_panelRect.height() : m_panelRect.width();
    const double base = m_metrics.thickness();
    const double thickness = drawn > 0 && base > 0 ? std::min(drawn, base) : (drawn > 0 ? drawn : base);
    return (thickness > 0 ? thickness : 1) * 28.0 / 86.0;
}

base::Rect DockWindow::baseWindowRect(int thickness) const
{
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return {};
    const base::Rect screen = output->bounds;
    if (m_dock->settings().efficientMode())
        return base::Rect(screen.x(), screen.bottom() - DockSettings::taskbarHeight,
                          std::max(1, screen.width() - m_taskbarRightWidth), DockSettings::taskbarHeight);
    const base::Rect available = m_shell->workArea(*output);
    const DockSettings &settings = m_dock->settings();
    const int margins = edgeGap();
    const bool bottom = settings.direction() == DockSettings::Bottom;
    const int length = (bottom ? screen.width() : available.height()) - 2 * margins;
    // The window reaches the screen edge, so the gap under the panel is the dock's.
    const int across = thickness + margins;
    switch (settings.direction()) {
    case DockSettings::Left:
        return base::Rect(screen.x(), available.y() + (available.height() - length) / 2, across,
                          length);
    case DockSettings::Right:
        return base::Rect(screen.x() + screen.width() - across,
                          available.y() + (available.height() - length) / 2, across, length);
    case DockSettings::Bottom:
        break;
    }
    return base::Rect(screen.x() + (screen.width() - length) / 2,
                      screen.y() + screen.height() - across, length, across);
}

base::Rect DockWindow::windowRect(int thickness) const
{
    const base::Rect rect = baseWindowRect(thickness);
    if (rect.isEmpty())
        return rect;
    // Room the magnified icons and a bounce need beyond the panel: the window
    // takes it on the screen's side, so the panel keeps its place against the
    // edge.
    const int headroom = magnificationHeadroom();
    if (headroom <= 0)
        return rect;
    switch (m_dock->settings().direction()) {
    case DockSettings::Left:
        return base::Rect(rect.x(), rect.y(), rect.width() + headroom, rect.height());
    case DockSettings::Right:
        return base::Rect(rect.x() - headroom, rect.y(), rect.width() + headroom, rect.height());
    case DockSettings::Bottom:
        break;
    }
    return base::Rect(rect.x(), rect.y() - headroom, rect.width(), rect.height() + headroom);
}

int DockWindow::magnificationHeadroom() const
{
    if (m_dock->settings().efficientMode()) return 0;
    const DockSettings &settings = m_dock->settings();
    const int iconSize = DockMetrics::forIconSize(settings.iconSize()).iconSize;
    const double grows = settings.magnification() ? iconSize * (settings.magnificationFactor() - 1.0) : 0.0;
    // A launch hop and an attention hop come from the same height, and either
    // may run, so the room is there for both.
    const double hops = iconSize * DockBounce::kAmplitude * (settings.bounceOnLaunch() ? 2.0 : 1.0);
    if (grows <= 0 && hops <= 0)
        return 0;
    return int(std::ceil(grows + hops)) + 2;
}

void DockWindow::configureMagnification()
{
    const DockSettings &settings = m_dock->settings();
    m_magnification->setEnabled(settings.magnification());
    m_magnification->setFactor(settings.magnificationFactor());
}

int DockWindow::edgeGap() const
{
    return m_dock->settings().edgeMargins() / 2;
}

render::GlassParams DockWindow::glass() const
{
    const double amount = glassAmount(m_dock->settings().glassTint());
    const auto blend = [amount](double clear, double middle, double tinted) {
        return glassBlend(amount, clear, middle, tinted);
    };
    render::GlassParams glass = fishuiGlass();
    // The glass blur kernel is 30 device pixels wide; a larger sigma truncates.
    glass.blurSigma = glassFixed(blend(0.6, 1.0, 8.0));
    glass.displacement = glassFixed(blend(36.0, 36.0, 0.0));
    // A colour split along the lip reads as a rainbow fringe next to the rim.
    glass.chroma = 0;
    glass.saturation = glassFixed(blend(1.4, 1.3, 1.0));
    glass.rim = glassFixed(1.0);
    // A grey tint as premultiplied value and alpha, blended apart.
    const bool dark = ui::Theme::current().darkMode;
    const double alpha = dark ? blend(0.12, 0.29, 0.50) : blend(0.16, 0.25, 0.53);
    const double value = (dark ? blend(0.046, 0.110, 0.190) : blend(0.060, 0.150, 0.530)) / alpha;
    // Rounded to 16 bits a component, then wl_fixed.
    const auto component = [](double v) {
        return glassFixed(std::round(std::clamp(v, 0.0, 1.0) * 65535.0) / 65535.0);
    };
    glass.tintRed = glass.tintGreen = glass.tintBlue = component(value);
    glass.tintAlpha = component(alpha);
    return glass;
}

std::vector<DockIcon *> DockWindow::icons() const
{
    std::vector<std::pair<double, DockIcon *>> ordered;
    for (const auto &[key, icon] : m_applicationIcons)
        ordered.push_back({icon->geometry().x + icon->geometry().y, icon});
    std::sort(ordered.begin(), ordered.end());
    std::vector<DockIcon *> result;
    for (const auto &[position, icon] : ordered)
        result.push_back(icon);
    std::vector<std::pair<double, DockIcon *>> minimized;
    for (const auto &[id, icon] : m_minimizedIcons)
        minimized.push_back({icon->geometry().x + icon->geometry().y, icon});
    std::sort(minimized.begin(), minimized.end());
    for (const auto &[position, icon] : minimized)
        result.push_back(icon);
    result.push_back(m_trash);
    return result;
}

DockIcon *DockWindow::applicationIcon(const std::string &id) const
{
    const auto it = m_applicationIcons.find(id);
    return it != m_applicationIcons.end() ? it->second : nullptr;
}

DockIcon *DockWindow::minimizedIcon(uint64_t window) const
{
    const auto it = m_minimizedIcons.find(window);
    return it != m_minimizedIcons.end() ? it->second : nullptr;
}

void DockWindow::rebuild()
{
    DockModel &applications = m_dock->applications();
    std::vector<std::string> keys;
    std::map<std::string, DockIcon *> icons;
    for (int row = 0; row < applications.count(); ++row) {
        const DockItem &item = applications.at(row);
        keys.push_back(item.id);
        DockIcon *icon = applicationIcon(item.id);
        if (!icon) {
            icon = m_panel->addChild<DockIcon>(this, DockIcon::Kind::Application);
            icon->setKey(item.id);
        }
        icon->setDraggable(!item.fixed);
        if (item.id == DockModel::kLauncherId)
            icon->setIcon(shellDataFile("launchpad.svg"), true);
        else
            icon->setIcon(item.iconName.empty() ? "application-x-desktop" : item.iconName);
        icon->setToolTip(item.visibleName);
        icon->setRunning(!item.windows.empty());
        icon->setActive(item.isActive);
        icon->setCount(m_shell->badges().count(item.id));
        icons[item.id] = icon;
    }
    // Icons that left stay while they shrink away; layoutItems() drops them.
    for (const auto &[key, icon] : m_applicationIcons) {
        if (!icons.contains(key))
            icons[key] = icon;
    }
    m_applicationIcons = std::move(icons);
    m_applications->setKeys(keys);

    MinimizedWindows &minimized = m_dock->minimizedWindows();
    std::vector<std::string> windowKeys;
    std::map<uint64_t, DockIcon *> windowIcons;
    for (int row = 0; row < minimized.count(); ++row) {
        const MinimizedWindows::Entry &entry = minimized.entries()[size_t(row)];
        windowKeys.push_back(std::to_string(entry.id));
        DockIcon *icon = minimizedIcon(entry.id);
        if (!icon) {
            icon = m_panel->addChild<DockIcon>(this, DockIcon::Kind::Minimized);
            icon->setKey(std::to_string(entry.id));
            icon->setWindowId(entry.id);
            icon->setDraggable(true);
        }
        // Its window can go before its row does; what leaves is still the
        // thumbnail it was.
        if (m_shell->taskWindows().window(entry.id)) {
            const std::string iconName = minimized.iconName(row);
            icon->setIcon(iconName.empty() ? "application-x-desktop" : iconName);
            icon->setToolTip(minimized.title(row));
            icon->setThumbnail(minimized.thumbnail(row), minimized.aspect(row));
            icon->setBadgeOpacity(entry.landed ? 1 : 0);
        }
        windowIcons[entry.id] = icon;
        m_minimizedStates[entry.id] = entry;
    }
    for (const auto &[id, icon] : m_minimizedIcons) {
        if (!windowIcons.contains(id))
            windowIcons[id] = icon;
    }
    m_minimizedIcons = std::move(windowIcons);
    m_minimized->setKeys(windowKeys);

    m_trash->setIcon(m_dock->trash().isEmpty() ? "user-trash-empty" : "user-trash-full");
    layoutItems();
}

void DockWindow::updateMetrics(bool animate)
{
    const DockSettings &settings = m_dock->settings();
    const DockModel &applications = m_dock->applications();
    const Shell::Output *output = m_shell->output({});
    const bool bottom = settings.direction() == DockSettings::Bottom;
    const int itemCount = applications.count() + 1 + m_dock->minimizedWindows().count();
    const int maxLength = m_dock->settings().efficientMode() && output
            ? std::max(1, output->bounds.width() - m_taskbarRightWidth) : output ? (bottom ? output->bounds.width()
                                           : m_shell->workArea(*output).height())
                                         - settings.edgeMargins()
                                 : 0;

    DockMetrics metrics = DockMetrics::forIconSize(settings.iconSize());
    // Not enough room for every icon: shrink until the dock fits, keeping
    // the paddings in proportion with the icons.
    const int separatorCount = applications.count() > applications.pinnedCount() ? 2 : 1;
    while (output && metrics.length(itemCount) + separatorCount * metrics.sectionExtent() > maxLength
           && metrics.iconSize > 1)
        metrics = DockMetrics::forIconSize(metrics.iconSize - 1);

    animate = animate && m_applications->animationsEnabled();
    const bool changed = metrics != m_metrics;
    if (changed) {
        // As drawn now, taken before the models restart the resize.
        m_fromEnds = m_metrics.iconSize + 2 * m_metrics.lengthPadding;
        m_fromMetrics = animate ? m_drawn : DrawnDockMetrics::of(metrics);
        m_metrics = metrics;
        m_applications->setPitch(metrics.pitch(), animate);
        m_minimized->setPitch(metrics.pitch(), animate);
        m_applications->setItemSize(metrics.iconSize);
        m_minimized->setItemSize(metrics.iconSize);
        if (!animate)
            m_fromEnds = -1;
    }
    m_applications->setSection(applications.pinnedCount(), metrics.sectionExtent(), animate);
}

void DockWindow::resize(bool animate)
{
    if (!m_dock->settings().isReady() && !m_windowRect.isEmpty())
        return;
    m_strutSettle.stop();
    updateMetrics(animate);
    const Shell::Output *output = m_shell->output({});
    if (output)
        m_window->setOutput(output->name);
    m_windowRect = windowRect(DockMetrics::forIconSize(m_dock->settings().iconSize()).thickness());
    if (!m_windowRect.isEmpty())
        m_window->setGeometry(m_windowRect);
    m_surfaceLength = m_dock->settings().direction() == DockSettings::Bottom ? m_windowRect.width()
                                                                             : m_windowRect.height();
    layoutItems();
    updateStrut();
    updateMinimizeTargets();
}

double DockWindow::place(double position, double rest) const
{
    // DockLayout::snap: moving in whole device pixels from where it comes to
    // rest, so it lands there while still moving rather than stepping on.
    const double ratio = pixelRatio();
    return rest + std::round((position - rest) * ratio) / ratio;
}

void DockWindow::updateLayoutStart()
{
    // DockLayout::update.
    const double ends = [this] {
        const double target = m_metrics.iconSize + 2 * m_metrics.lengthPadding;
        if (m_fromEnds < 0)
            return target;
        return m_fromEnds + (target - m_fromEnds) * m_applications->resizeProgress();
    }();
    m_minimizedStart = m_applications->length() + m_applications->trailingExtent();
    m_length = m_minimizedStart + m_minimized->length() + ends;
    const double resting = m_applications->targetLength() + m_applications->targetTrailingExtent()
            + m_minimized->targetLength() + m_metrics.iconSize + 2 * m_metrics.lengthPadding;
    const auto restingStart = [this](double length) {
        return std::floor((m_surfaceLength - length) / 2);
    };
    if (resting != m_restingLength || m_surfaceLength != m_restingSurface) {
        // Heading somewhere new: the offset travels there as the length does.
        m_fromLength = m_length;
        m_fromBias = m_restingLength < 0 ? restingStart(resting) - (m_surfaceLength - resting) / 2
                                         : m_bias;
        m_toBias = restingStart(resting) - (m_surfaceLength - resting) / 2;
        m_restingLength = resting;
        m_restingSurface = m_surfaceLength;
    }
    // A full dock barely changes length as its icons resize: the offset follows the resize.
    const double resize = m_applications->resizeProgress();
    const double distance = m_restingLength - m_fromLength;
    const double progress = resize < 1                ? resize
            : std::abs(distance) < 1e-9 ? 1
                                        : std::clamp((m_length - m_fromLength) / distance, 0.0, 1.0);
    m_bias = m_fromBias + (m_toBias - m_fromBias) * progress;
    m_start = m_dock->settings().efficientMode() ? 0 : (m_surfaceLength - m_length) / 2 + m_bias;
    if (resize >= 1)
        m_fromEnds = -1;
}

void DockWindow::layoutItems()
{
    if (m_windowRect.isEmpty() || m_layingOut)
        return;
    m_layingOut = true;
    updateLayoutStart();
    const DockSettings::Direction direction = m_dock->settings().direction();
    const bool bottom = direction == DockSettings::Bottom;
    const bool left = direction == DockSettings::Left;

    // Everything between two sizes while icons come or go on a full dock.
    m_drawn = DrawnDockMetrics::blend(m_fromMetrics, DrawnDockMetrics::of(m_metrics),
                                      m_applications->resizeProgress());
    const double drawnThickness = m_dock->settings().efficientMode() ? DockSettings::taskbarHeight : std::round(m_drawn.thickness);
    const double farPadding = m_drawn.outerPadding + drawnThickness - m_drawn.thickness;
    const double dotArea = m_drawn.edgePadding + m_drawn.dotSize + m_drawn.dotGap;
    const double itemSize = m_metrics.iconSize;
    const double itemScale = m_drawn.iconSize / itemSize;
    const double itemGrowth = (m_drawn.iconSize - itemSize) / 2;

    const bool efficient = m_dock->settings().efficientMode();
    const int panelStart = efficient ? 0 : int(std::floor(m_start + 0.5));
    const int panelEnd = efficient ? m_windowRect.width() : int(std::floor(m_start + m_length + 0.5));
    const int gap = edgeGap();
    const int farSide = (bottom ? m_windowRect.height() : m_windowRect.width()) - gap;
    const base::Rect panel = bottom
            ? base::Rect(panelStart, farSide - int(drawnThickness), panelEnd - panelStart,
                         int(drawnThickness))
            : base::Rect(left ? gap : farSide - int(drawnThickness), panelStart,
                         int(drawnThickness), panelEnd - panelStart);

    // The content's origin along the dock, and an item's across from the panel's far side.
    const double origin = m_drawn.lengthPadding + m_start - panelStart;
    const double layoutOrigin = m_start + m_drawn.lengthPadding;
    const double settledStart = m_dock->settings().efficientMode() ? 0 : std::floor((m_surfaceLength - m_restingLength) / 2);
    const double settled = settledStart + m_metrics.lengthPadding;
    const auto along = [&](double position, double rest) {
        return place(layoutOrigin + position, settled + rest) - layoutOrigin;
    };
    const double across = bottom ? farPadding : left ? gap + dotArea : farPadding;

    // An item's slot in panel coordinates from its position along the content.
    const auto slotRect = [&](double position) {
        const double start = origin + position;
        if (bottom)
            return ui::RectF {start - m_drawn.iconGap / 2, 0, itemSize + m_drawn.iconGap,
                              drawnThickness + gap};
        return ui::RectF {left ? -double(gap) : 0, start - m_drawn.iconGap / 2,
                          drawnThickness + gap, itemSize + m_drawn.iconGap};
    };
    // The icon inside a slot, in the item's own coordinates.
    const ui::RectF iconIn = bottom ? ui::RectF {m_drawn.iconGap / 2, across, itemSize, itemSize}
                                    : ui::RectF {across, m_drawn.iconGap / 2, itemSize, itemSize};

    // Where each item goes and what it is drawn as, before the magnification
    // and the bounce have had their say.
    struct Placed
    {
        DockIcon *icon;
        double position;
        double scale;
        double opacity;
    };
    std::vector<Placed> placed;
    const auto place = [&](DockIcon *icon, double position, double scale, double opacity) {
        placed.push_back({icon, position, scale, opacity});
        m_leavingIcons.erase(icon);
        icon->setEnabled(true);
    };

    // Out of the layout, above the icons that close up under it, where it was
    // last placed.
    const auto leave = [&](DockIcon *icon, double opacity) {
        if (m_leavingIcons.insert(icon).second)
            m_panel->raiseChild(icon);
        icon->setPresentationOpacity(opacity);
        icon->setEnabled(false);
    };
    const auto staying = [](const DockPresentation &presentation) {
        std::set<std::string> keys;
        for (const DockPresentation::Entry &entry : presentation.entries()) {
            if (!entry.leaving)
                keys.insert(entry.key);
        }
        return keys;
    };

    std::set<std::string> present;
    const std::set<std::string> stayingApplications = staying(*m_applications);
    for (const DockPresentation::Entry &entry : m_applications->entries()) {
        DockIcon *icon = applicationIcon(entry.key);
        if (!icon)
            continue;
        present.insert(entry.key);
        if (entry.leaving) {
            if (!stayingApplications.contains(entry.key)) {
                leave(icon, m_applications->appearance(entry));
                icon->setRunning(false);
            }
            continue;
        }
        const double position = along(m_applications->position(entry), m_applications->targetPosition(entry))
                + itemGrowth;
        place(icon, position, m_applications->iconScale(entry), m_applications->appearance(entry));
    }
    for (auto it = m_applicationIcons.begin(); it != m_applicationIcons.end();) {
        if (present.contains(it->first)) {
            ++it;
            continue;
        }
        if (m_tooltipOwner == it->second)
            hideTooltip();
        if (m_menuIcon == it->second)
            m_menuIcon = nullptr;
        if (m_drop.hovered == it->second)
            setDropHovered(nullptr);
        retire(it->second);
        it = m_applicationIcons.erase(it);
    }

    std::set<uint64_t> windows;
    const double minimizedStart = m_minimizedStart;
    const double settledMinimizedStart = m_applications->targetLength() + m_applications->targetTrailingExtent();
    const std::set<std::string> stayingWindows = staying(*m_minimized);
    for (const DockPresentation::Entry &entry : m_minimized->entries()) {
        const uint64_t id = std::stoull(entry.key);
        DockIcon *icon = minimizedIcon(id);
        if (!icon)
            continue;
        windows.insert(id);
        // A window that flies in arrives in its slot itself and becomes its
        // thumbnail whole; one shown again from hidden is revealed like an
        // icon, and a closed one fades out where it was.
        const auto state = m_minimizedStates.find(id);
        const bool landed = state == m_minimizedStates.end() || state->second.landed;
        if (entry.leaving) {
            if (!stayingWindows.contains(entry.key))
                leave(icon, landed ? m_minimized->appearance(entry) : 0);
            continue;
        }
        const double position = along(minimizedStart + m_minimized->position(entry),
                                      settledMinimizedStart + m_minimized->targetPosition(entry))
                + itemGrowth;
        const bool revealed = state == m_minimizedStates.end() || !state->second.flownIn;
        place(icon, position, revealed ? m_minimized->iconScale(entry) : 1,
              !landed ? 0 : revealed ? m_minimized->appearance(entry) : 1);
    }
    for (auto it = m_minimizedIcons.begin(); it != m_minimizedIcons.end();) {
        if (windows.contains(it->first)) {
            ++it;
            continue;
        }
        if (m_tooltipOwner == it->second)
            hideTooltip();
        if (m_menuIcon == it->second)
            m_menuIcon = nullptr;
        if (m_drop.hovered == it->second)
            setDropHovered(nullptr);
        retire(it->second);
        m_minimizedStates.erase(it->first);
        it = m_minimizedIcons.erase(it);
    }

    m_trash->setVisible(!efficient);
    if (!efficient)
        place(m_trash,
              along(minimizedStart + m_minimized->length(), settledMinimizedStart + m_minimized->targetLength())
                      + itemGrowth,
              1, 1);

    // The row as it rests, for the magnification: every icon's centre along
    // the dock and its width. Storage order need not match spatial order.
    std::vector<DockMagnification::Item> items;
    items.reserve(placed.size());
    for (size_t i = 0; i < placed.size(); ++i) {
        const double centre = placed[i].position + itemSize / 2;
        items.push_back({centre, itemSize});
    }
    m_magnification->setItems(std::move(items), m_metrics.pitch());
    const double contentOrigin = m_start + m_drawn.lengthPadding;
    // A drag from the dock, or one arriving over it, holds the icons still.
    const bool dragging = m_drag.has_value() || m_drop.offer;
    if (m_pointerAlong && !dragging)
        m_magnification->setPointer(*m_pointerAlong - contentOrigin);
    else
        m_magnification->setPointer(std::nullopt);

    // Drawing: the rest places, moved by the magnification's shifts and scaled
    // by it, and lifted off the dock's base line by the magnification and the
    // bounce.
    const std::vector<DockMagnification::Placement> &placements = m_magnification->placements();
    const bool smoothed = placements.size() == placed.size();
    const double restIconSize = itemSize * itemScale;
    const auto liftOff = [&](double lift) {
        if (bottom)
            return ui::PointF {0, -lift};
        return ui::PointF {left ? lift : -lift, 0};
    };
    const auto include = [](std::optional<ui::RectF> &bounds, const ui::RectF &rect) {
        if (!bounds) {
            bounds = rect;
            return;
        }
        const double left = std::min(bounds->x, rect.x);
        const double top = std::min(bounds->y, rect.y);
        const double right = std::max(bounds->x + bounds->width, rect.x + rect.width);
        const double bottom = std::max(bounds->y + bounds->height, rect.y + rect.height);
        bounds = ui::RectF {left, top, right - left, bottom - top};
    };

    bool lifted = false;
    std::optional<ui::RectF> rest;
    std::optional<ui::RectF> drawn;
    for (size_t i = 0; i < placed.size(); ++i) {
        const double scale = smoothed ? placements[i].scale : 1;
        const double shift = smoothed ? placements[i].shift : 0;
        const double hop = m_bounce->lift(placed[i].icon) + m_attentionBounce->lift(placed[i].icon);
        const double lift = (scale - 1) * itemSize / 2 + hop;
        const ui::PointF offset = liftOff(lift);
        if (scale != 1 || lift != 0)
            lifted = true;

        const double position = placed[i].position + shift;
        const ui::RectF itSlot = slotRect(position);
        const ui::RectF restSlot = slotRect(placed[i].position);
        const double restCentreX = restSlot.x + panel.x() + iconIn.x + iconIn.width / 2;
        const double restCentreY = restSlot.y + panel.y() + iconIn.y + iconIn.height / 2;
        const double drawnCentreX = itSlot.x + panel.x() + iconIn.x + iconIn.width / 2 + offset.x;
        const double drawnCentreY = itSlot.y + panel.y() + iconIn.y + iconIn.height / 2 + offset.y;
        const double size = restIconSize * scale;
        include(rest, ui::RectF {restCentreX - restIconSize / 2, restCentreY - restIconSize / 2,
                                 restIconSize, restIconSize});
        include(drawn, ui::RectF {drawnCentreX - size / 2, drawnCentreY - size / 2, size, size});

        placed[i].icon->setGeometry({itSlot.x + panel.x(), itSlot.y + panel.y(), itSlot.width,
                                     itSlot.height});
        placed[i].icon->setIconRect(iconIn);
        placed[i].icon->setPresentation(placed[i].scale * scale, placed[i].opacity, offset);
        const double dot = m_metrics.dotSize;
        const double dotGap = m_metrics.dotGap;
        if (bottom)
            placed[i].icon->setDotRect({iconIn.x + (itemSize - dot) / 2, iconIn.y + itemSize + dotGap, dot, dot});
        else if (left)
            placed[i].icon->setDotRect({iconIn.x - (dotGap + dot), iconIn.y + (itemSize - dot) / 2, dot, dot});
        else
            placed[i].icon->setDotRect({iconIn.x + itemSize + dotGap, iconIn.y + (itemSize - dot) / 2, dot, dot});
    }

    // The panel the icons need: it grows along the dock with the spread, but
    // keeps its rest thickness across it. Magnified icons then leave the bar
    // on the screen's side, as they do on macOS, while its edge stays put.
    base::Rect panelRect = panel;
    if (rest && drawn) {
        double extraLeft = 0;
        double extraTop = 0;
        double extraRight = 0;
        double extraBottom = 0;
        if (bottom) {
            extraLeft = std::min(0.0, drawn->x - rest->x);
            extraRight = std::max(0.0, drawn->x + drawn->width - (rest->x + rest->width));
        } else {
            extraTop = std::min(0.0, drawn->y - rest->y);
            extraBottom = std::max(0.0, drawn->y + drawn->height - (rest->y + rest->height));
        }
        if (extraLeft != 0 || extraTop != 0 || extraRight != 0 || extraBottom != 0) {
            const double leftEdge = panel.x() + extraLeft;
            const double topEdge = panel.y() + extraTop;
            const double rightEdge = panel.x() + panel.width() + extraRight;
            const double bottomEdge = panel.y() + panel.height() + extraBottom;
            const int rectLeft = std::clamp(int(std::floor(leftEdge)), 0, m_windowRect.width());
            const int rectTop = std::clamp(int(std::floor(topEdge)), 0, m_windowRect.height());
            const int rectRight =
                    std::clamp(int(std::ceil(rightEdge)), rectLeft + 1, m_windowRect.width());
            const int rectBottom =
                    std::clamp(int(std::ceil(bottomEdge)), rectTop + 1, m_windowRect.height());
            panelRect = base::Rect(rectLeft, rectTop, rectRight - rectLeft, rectBottom - rectTop);
        }
    }

    // The dividers: 48/86 of the thickness, centred across the panel, in the
    // middle of their gap with the leading edge half a pixel before it.
    const double lineLength = std::round(drawnThickness * 48 / 86);
    const double lineAcross = std::round((drawnThickness - lineLength) / 2);
    const double middle = (m_drawn.separatorExtent - m_drawn.iconGap) / 2 - 0.5;
    const auto lineRect = [&](double position) {
        const double start = origin + position;
        return bottom ? base::RectF(start, lineAcross, 1, lineLength)
                      : base::RectF(lineAcross, start, lineLength, 1);
    };

    // During magnification the icons shift laterally. The separator must
    // shift with the pinned icons that precede it: find the cumulative shift
    // at the section boundary from the last placed icon whose rest position
    // is before (or at) sectionPosition.
    const double sectionPos = m_applications->sectionPosition();
    double separatorShift = 0;
    if (smoothed) {
        // Walk placed in reverse; find the last one at or before sectionPos.
        for (int i = int(placed.size()) - 1; i >= 0; --i) {
            if (placed[i].position <= sectionPos + itemSize / 2) {
                separatorShift = placements[i].shift;
                break;
            }
        }
    }

    m_divider = lineRect(along(sectionPos + middle,
                               m_applications->targetSectionPosition() + middle)
                         + separatorShift);
    const double extent = m_applications->trailingExtent();
    m_trailingOpacity = 0.18 * std::clamp((extent - 2) / (m_drawn.separatorExtent - 2), 0.0, 1.0);

    // The trailing divider sits at the end of the applications section.
    // Its shift follows the last placed application icon.
    double trailingSeparatorShift = 0;
    if (smoothed && !placed.empty())
        trailingSeparatorShift = placements[placed.size() - 1].shift;

    m_trailingDivider = m_trailingOpacity > 0
            ? lineRect(along(m_applications->length() + middle * extent / m_drawn.separatorExtent,
                             m_applications->targetLength()
                                     + middle * m_applications->targetTrailingExtent()
                                             / m_metrics.sectionExtent())
                       + trailingSeparatorShift)
            : base::RectF();
    m_panel->setGeometry({0, 0, double(m_windowRect.width()), double(m_windowRect.height())});
    m_panel->requestPaint();
    updatePanelRect(panelRect, !lifted);
    // Landed windows follow their sliding thumbnails.
    updateMinimizeTargets();
    if (m_tooltipOwner)
        showTooltip(m_tooltipOwner);
    m_layingOut = false;
}

void DockWindow::retire(DockIcon *icon)
{
    m_bounce->stop(icon);
    m_attentionBounce->stop(icon);
    // Not from inside the frame that laid it out: destroyed on the next turn.
    icon->setVisible(false);
    m_leavingIcons.erase(icon);
    m_retired.push_back(m_panel->takeChild(icon));
    if (m_retired.size() == 1) {
        core::mainLoop()->post([this, alive = m_alive] {
            if (*alive)
                m_retired.clear();
        });
    }
}

void DockWindow::updatePanelRect(const base::Rect &rect, bool notify)
{
    if (rect == m_panelRect)
        return;
    m_panelRect = rect;
    updateGlass();
    const DockSettings::Direction direction = m_dock->settings().direction();
    const base::Rect gap = direction == DockSettings::Bottom
            ? base::Rect::fromEdges(rect.x(), rect.y() + rect.height() / 2, rect.right(),
                                    m_windowRect.height())
            : direction == DockSettings::Left
            ? base::Rect::fromEdges(0, rect.y(), rect.x() + rect.width() / 2, rect.bottom())
            : base::Rect::fromEdges(rect.x() + rect.width() / 2, rect.y(), m_windowRect.width(),
                                    rect.bottom());
    m_window->setInputRegion(roundedRegion(rect, cornerRadius()) | gap);
    // While the icons magnify the panel moves every frame; the dock's place on
    // the screen is still its rest one, so only a settled panel announces it.
    if (notify)
        m_dock->notifyPrimaryGeometryChanged();
}

void DockWindow::panelHoverMoved(const ui::PointF &position)
{
    const DockSettings::Direction direction = m_dock->settings().direction();
    setPointerAlong(direction == DockSettings::Bottom ? position.x : position.y);
}

void DockWindow::panelHoverLeft()
{
    setPointerAlong(std::nullopt);
}

void DockWindow::setPointerAlong(std::optional<double> along)
{
    if (m_pointerAlong == along)
        return;
    m_pointerAlong = along;
    // The magnification turns the pointer into scales and shifts on the next
    // layout, and runs on from there frame by frame.
    layoutItems();
}

void DockWindow::iconRunningChanged(DockIcon *icon, bool running)
{
    // The window arrived: there is nothing left to wait for.
    if (running)
        m_bounce->stop(icon);
}

void DockWindow::attentionChanged(uint64_t window)
{
    if (m_dock->settings().efficientMode()) return;
    TaskWindows &windows = m_shell->taskWindows();
    const TaskWindow *info = windows.window(window);
    if (!info)
        return;
    std::string application = m_dock->applications().appIdForWindow(window);
    if (application.empty())
        application = info->appId;
    DockIcon *icon = applicationIcon(application);
    if (!icon)
        return;
    if (!info->demandsAttention) {
        // Another window of the same application may still ask for it.
        for (uint64_t other : windows.windows()) {
            const TaskWindow *candidate = windows.window(other);
            if (candidate && candidate->demandsAttention && candidate->appId == info->appId)
                return;
        }
        m_attentionBounce->stop(icon);
        return;
    }
    m_attentionBounce->start(icon, m_metrics.iconSize, DockBounce::kAttentionHops);
}

void DockWindow::updateGlass()
{
    if (m_panelRect.isEmpty())
        return;
    // GlassEffect's radius: whole pixels, at most half the region.
    const double radius = std::min(cornerRadius(),
                                   double(std::min(m_panelRect.width(), m_panelRect.height())) / 2);
    m_glassDark = ui::Theme::current().darkMode;
    m_window->setBackgroundBlur(m_panelRect, int(std::lround(radius)), glass());
}

void DockWindow::updateStrut()
{
    const DockSettings &settings = m_dock->settings();
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return;
    // The reserved strip runs from the screen edge to the far side of the
    // dock, so it covers the dock plus its edge gap.
    if (settings.efficientMode()) {
        m_window->setExclusiveZone(wm::ScreenEdge::Bottom, DockSettings::taskbarHeight);
        return;
    }
    constexpr int kStrutExtraPixels = 2;
    const base::Rect screen = output->bounds;
    const base::Rect dock = baseWindowRect(m_metrics.thickness());
    int thickness = 0;
    wm::ScreenEdge edge = wm::ScreenEdge::Bottom;
    switch (settings.direction()) {
    case DockSettings::Left:
        edge = wm::ScreenEdge::Left;
        thickness = (dock.x() + dock.width() - 1) - screen.x() + kStrutExtraPixels;
        break;
    case DockSettings::Right:
        edge = wm::ScreenEdge::Right;
        thickness = (screen.x() + screen.width() - 1) - dock.x() + kStrutExtraPixels;
        break;
    case DockSettings::Bottom:
        thickness = (screen.y() + screen.height() - 1) - dock.y() + kStrutExtraPixels;
        break;
    }
    switch (settings.visibility()) {
    case DockSettings::AlwaysShow:
        m_window->setExclusiveZone(edge, thickness);
        break;
    case DockSettings::IntellHide:
        m_window->setExclusiveZone(edge, thickness, true);
        break;
    case DockSettings::AlwaysHide:
        m_window->setExclusiveZone(edge, 0);
        break;
    }
}

base::Rect DockWindow::minimizedSlot(int index, int count) const
{
    const double pitch = m_metrics.pitch();
    const double settled = m_applications->targetLength() + m_applications->targetTrailingExtent();
    const double before = settled + index * pitch;
    const double start = std::floor((m_surfaceLength - (settled + count * pitch + m_metrics.iconSize
                                                        + 2 * m_metrics.lengthPadding))
                                    / 2);
    const DockSettings::Direction direction = m_dock->settings().direction();
    const int thickness = m_metrics.thickness();
    const int size = m_metrics.iconSize;
    const int gap = edgeGap();
    if (direction == DockSettings::Bottom)
        return base::Rect(int(start + m_metrics.lengthPadding + before),
                          m_windowRect.height() - gap - thickness + m_metrics.outerPadding, size, size);
    return base::Rect(direction == DockSettings::Left
                              ? gap + m_metrics.dotAreaSize()
                              : m_windowRect.width() - gap - thickness + m_metrics.outerPadding,
                      int(start + m_metrics.lengthPadding + before), size, size);
}

base::Rect DockWindow::applicationSlot(double position) const
{
    const double settledStart = m_dock->settings().efficientMode() ? 0 : std::floor((m_surfaceLength - m_restingLength) / 2);
    const double alongStart = settledStart + m_metrics.lengthPadding + position;
    const DockSettings::Direction direction = m_dock->settings().direction();
    const int thickness = m_metrics.thickness();
    const int size = m_metrics.iconSize;
    const int gap = edgeGap();
    if (direction == DockSettings::Bottom)
        return base::Rect(int(alongStart),
                          m_windowRect.height() - gap - thickness + m_metrics.outerPadding, size, size);
    return base::Rect(direction == DockSettings::Left
                              ? gap + m_metrics.dotAreaSize()
                              : m_windowRect.width() - gap - thickness + m_metrics.outerPadding,
                      int(alongStart), size, size);
}

void DockWindow::updateMinimizeTargets()
{
    wm::Window *panel = m_window->wmWindow();
    if (!panel)
        return;
    wm::WindowManager *manager = m_shell->windowManager();
    TaskWindows &windows = m_shell->taskWindows();
    MinimizedWindows &minimized = m_dock->minimizedWindows();
    const bool area = m_dock->settings().minimizedArea();
    // Every window aims at the trash's place, where the next thumbnail opens.
    const base::Rect next = minimizedSlot(minimized.count(), minimized.count() + 1);
    for (uint64_t id : windows.windows()) {
        const TaskWindow *window = windows.window(id);
        if (!window || !window->isTaskListWindow())
            continue;
        base::Rect target;
        if (!area) {
            // Into its application's icon, which it also restores from.
            const std::string application = m_dock->applications().appIdForWindow(id);
            target = next;
            for (const DockPresentation::Entry &entry : m_applications->entries()) {
                if (entry.key == application && !entry.leaving)
                    target = applicationSlot(m_applications->targetPosition(entry));
            }
        } else if (window->minimized) {
            const auto &entries = minimized.entries();
            const auto entry = std::find_if(entries.begin(), entries.end(),
                                            [id](const MinimizedWindows::Entry &e) { return e.id == id; });
            const DockIcon *icon = minimizedIcon(id);
            if (entry == entries.end() || !icon)
                continue;
            if (!entry->landed) {
                // Still flying: aimed at where its slot comes to rest, not at
                // the slot opening under it, which would move the landing.
                target = minimizedSlot(int(entry - entries.begin()), minimized.count());
            } else {
                // Landed: follows its thumbnail, which it restores from.
                const ui::RectF rect = icon->iconRect();
                target = base::Rect(int(std::lround(icon->geometry().x + rect.x)),
                                    int(std::lround(icon->geometry().y + rect.y)), int(rect.width),
                                    int(rect.height));
            }
        } else {
            target = next;
        }
        const base::Rect fitted = fitToWindow(target, window->geometry);
        if (fitted.isEmpty() || m_sentTargets[id] == fitted)
            continue;
        m_sentTargets[id] = fitted;
        if (wm::Window *wmWindow = manager->windowById(id))
            wmWindow->setMinimizedGeometry(panel, fitted);
    }
}

void DockWindow::handOffLanded()
{
    if (m_landed.empty())
        return;
    wm::WindowManager *manager = m_shell->windowManager();
    for (uint64_t id : std::exchange(m_landed, {})) {
        if (wm::Window *window = manager->windowById(id))
            window->endMinimizeHandoff();
    }
}

void DockWindow::iconHovered(DockIcon *icon, bool hovered)
{
    if (hovered) {
        m_tooltipHide.stop();
        showTooltip(icon);
        return;
    }
    // The next item's enter arrives before this leave now that the slots
    // touch, and it already owns the tooltip.
    if (m_tooltipOwner == icon)
        m_tooltipHide.start();
}

void DockWindow::showTooltip(DockIcon *icon)
{
    // A menu holds the interaction while it is open.
    if (m_menuOpen || icon->toolTipText().empty())
        return;
    m_tooltipOwner = icon;
    // Above the icon as drawn, so a magnified one keeps its name over it.
    const ui::RectF rect = icon->presentedIconRect();
    const ui::PointF centre = icon->mapToWindow({rect.x + rect.width / 2, rect.y + rect.height / 2});
    m_tooltip->show(icon->toolTipText(), base::PointF {centre.x, centre.y});
}

// Menus are made when they open: an open one closes.
void DockWindow::retranslate()
{
    if (m_menu)
        m_menu->close();
    hideTooltip();
    m_trash->setToolTip(tr("Trash"));
    m_dock->applications().retranslate();
}

void DockWindow::hideTooltip()
{
    m_tooltipHide.stop();
    m_tooltipOwner = nullptr;
    if (m_tooltip)
        m_tooltip->hide();
}

void DockWindow::iconClicked(DockIcon *icon, int button)
{
    switch (icon->kind()) {
    case DockIcon::Kind::Application:
        if (button == int(ui::MouseButton::Left)) {
            m_dock->applications().clicked(icon->key());
            // Starting, not switching to: it hops until its window is there.
            if (m_dock->settings().bounceOnLaunch() && !icon->isRunning()
                && icon->key() != DockModel::kLauncherId)
                m_bounce->start(icon, m_metrics.iconSize);
        } else if (button == int(ui::MouseButton::Middle)) {
            m_dock->applications().openNewInstance(icon->key());
        }
        break;
    case DockIcon::Kind::Minimized:
        if (button == int(ui::MouseButton::Left))
            m_dock->minimizedWindows().restore(icon->windowId());
        break;
    case DockIcon::Kind::Trash:
        if (button == int(ui::MouseButton::Left)) {
            callLauncher(m_shell->bus(), "hide");
            TrashMonitor::openTrash(m_shell->bus());
        }
        break;
    }
}

void DockWindow::iconMenuRequested(DockIcon *icon)
{
    auto menu = std::make_unique<ui::Menu>();
    base::dbus::Connection *bus = m_shell->bus();
    switch (icon->kind()) {
    case DockIcon::Kind::Application: {
        const std::string id = icon->key();
        // The launcher has no menu.
        if (id == DockModel::kLauncherId)
            return;
        DockModel &model = m_dock->applications();
        const int row = model.indexOf(id);
        if (row < 0)
            return;
        const DockItem &item = model.at(row);
        const auto windows = model.windows(id);
        for (const DockModel::WindowInfo &window : windows) {
            const size_t index = menu->addItem(window.title.empty() ? item.visibleName : window.title,
                                               [&model, id = window.id] { model.activateWindow(id); });
            menu->setItemChecked(index, window.active);
            // A long title is elided instead of widening the menu.
            menu->setItemMaximumWidth(index, 320);
        }
        if (!windows.empty())
            menu->addSeparator();
        if (model.canOpenNewInstance(id))
            menu->addItem(tr("New Window"), [&model, id] { model.openNewInstance(id); });
        else if (windows.empty())
            menu->addItem(tr("Open"), [&model, id] { model.openNewInstance(id); });
        if (item.pinnable()) {
            const bool pinned = item.isPinned;
            menu->addItem(pinned ? tr("Unpin") : tr("Pin"), [&model, id, pinned] {
                pinned ? model.unPin(id) : model.pin(id);
            });
        }
        if (!windows.empty())
            menu->addItem(tr("Quit"), [&model, id] { model.quit(id); });
        break;
    }
    case DockIcon::Kind::Minimized: {
        const uint64_t window = icon->windowId();
        MinimizedWindows &minimized = m_dock->minimizedWindows();
        menu->addItem(tr("Restore"), [&minimized, window] { minimized.restore(window); });
        menu->addItem(tr("Close"), [&minimized, window] { minimized.close(window); });
        break;
    }
    case DockIcon::Kind::Trash: {
        menu->addItem(tr("Open"), [bus] {
            callLauncher(bus, "hide");
            TrashMonitor::openTrash(bus);
        });
        const size_t empty = menu->addItem(tr("Empty Trash"), [bus] { TrashMonitor::emptyTrash(bus); });
        menu->setItemVisible(empty, !m_dock->trash().isEmpty());
        break;
    }
    }
    popupMenu(icon, std::move(menu));
}

void DockWindow::popupMenu(DockIcon *icon, std::unique_ptr<ui::Menu> menu)
{
    hideTooltip();
    if (m_menu)
        m_menu->close();
    if (m_menuIcon)
        m_menuIcon->setMenuOpen(false);
    m_menu = std::move(menu);
    m_menuIcon = icon;
    m_menu->onClosed = [this] {
        m_menuOpen = false;
        DockIcon *icon = std::exchange(m_menuIcon, nullptr);
        if (!icon)
            return;
        icon->setMenuOpen(false);
        // Back over the icon it was opened from: its tooltip again.
        if (icon->isPointerOver())
            showTooltip(icon);
    };
    // DockItem.popupMenu(): the arrow at the icon's centre, the menu a
    // small spacing off the panel's far side, starting at the icon's leading
    // side and pulled back only as far as the arrow needs.
    const DockSettings::Direction direction = m_dock->settings().direction();
    const ui::RectF slot = icon->geometry();
    const ui::RectF rect = icon->presentedIconRect();
    const ui::PointF iconStart {slot.x + rect.x, slot.y + rect.y};
    const ui::PointF centre {iconStart.x + rect.width / 2, iconStart.y + rect.height / 2};
    m_menu->setArrow(direction == DockSettings::Left    ? ui::BubbleShape::Edge::Left
                             : direction == DockSettings::Right ? ui::BubbleShape::Edge::Right
                                                                : ui::BubbleShape::Edge::Bottom,
                     centre);
    m_menu->setContinuousCorners(true);
    if (const Shell::Output *output = m_shell->output({})) {
        base::Rect area = m_shell->workArea(*output);
        area.translate(-m_windowRect.x(), -m_windowRect.y());
        m_menu->setAvailableGeometry(
                ui::RectF {double(area.x()), double(area.y()), double(area.width()), double(area.height())});
    }
    ui::Window *window = m_window->window();
    const ui::SizeF size = m_menu->implicitSize(window);
    const double gap = ui::Theme::current().smallSpacing;
    ui::PointF origin;
    if (direction == DockSettings::Left)
        origin = {double(m_panelRect.x() + m_panelRect.width()) + gap, centre.y - size.height / 2};
    else if (direction == DockSettings::Right)
        origin = {m_panelRect.x() - size.width - gap, centre.y - size.height / 2};
    else
        origin = {centre.x - size.width / 2, m_panelRect.y() - size.height - gap};
    const double inset = m_menu->minimumEdgeLength() / 2;
    if (direction == DockSettings::Bottom)
        origin.x = std::min(iconStart.x, centre.x - inset);
    else
        origin.y = std::min(iconStart.y, centre.y - inset);
    m_menu->popupAt(window, {std::round(origin.x), std::round(origin.y)});
    m_menuOpen = m_menu->isOpen();
    icon->setMenuOpen(m_menuOpen);
}

void DockWindow::iconDragRequested(DockIcon *icon)
{
    if (m_drag || icon->kind() == DockIcon::Kind::Trash)
        return;
    hideTooltip();
    // Drag.onDragStarted: the press no longer dims it.
    icon->releasePress();
    double scale = 1;
    ui::Bitmap image = iconImage(icon, &scale);

    IconDrag drag;
    drag.kind = icon->kind();
    drag.key = icon->key();
    drag.window = icon->windowId();
    const bool application = drag.kind == DockIcon::Kind::Application;
    ui::DragSource source;
    source.mimeTypes = {application ? kApplicationMime : kWindowMime};
    source.data = [key = drag.key](const std::string &) { return key; };
    source.actions = ui::MoveAction;
    source.icon = std::move(image);
    source.iconScale = scale;
    const ui::RectF rect = icon->iconRect();
    source.hotSpot = {rect.width / 2, rect.height / 2};
    // A drag the compositor never ran ends here; one it ran, in onDragEnded().
    source.onFinished = [this, alive = m_alive](ui::DropAction) {
        if (*alive && m_drag && !m_drag->started)
            finishIconDrag();
    };
    m_drag = drag;
    icon->setDragHidden(true);
    if (!icon->startDrag(std::move(source)) && m_drag && !m_drag->started)
        finishIconDrag();
}

ui::Bitmap DockWindow::iconImage(DockIcon *icon, double *scale) const
{
    // The icon as drawn in its slot at the press.
    *scale = pixelRatio();
    const ui::Image window = m_window->window()->grab();
    const ui::RectF slot = icon->geometry();
    const ui::RectF rect = icon->presentedIconRect();
    const int x = int(std::lround((slot.x + rect.x) * *scale));
    const int y = int(std::lround((slot.y + rect.y) * *scale));
    const int width = int(std::lround(rect.width * *scale));
    const int height = int(std::lround(rect.height * *scale));
    if (window.isNull() || x < 0 || y < 0 || x + width > window.width() || y + height > window.height()
        || width <= 0 || height <= 0)
        return {};
    return ui::Bitmap::copyOf(window.bits() + size_t(y) * window.width() + x, width, height,
                              size_t(window.width()) * 4, ui::Bitmap::Format::Argb32Premultiplied);
}

base::PointF DockWindow::iconCentre(DockIcon *icon) const
{
    const ui::RectF slot = icon->geometry();
    const ui::RectF rect = icon->iconRect();
    return {m_windowRect.x() + slot.x + rect.x + rect.width / 2,
            m_windowRect.y() + slot.y + rect.y + rect.height / 2};
}

double DockWindow::distanceOffDock(const base::PointF &position) const
{
    const base::Rect panel = primaryGeometry();
    switch (m_dock->settings().direction()) {
    case DockSettings::Left:
        return position.x() - (panel.x() + panel.width());
    case DockSettings::Right:
        return panel.x() - position.x();
    case DockSettings::Bottom:
        break;
    }
    return panel.y() - position.y();
}

void DockWindow::onDragStarted()
{
    input::DragController *drag = m_shell->seat()->drag();
    if (!m_drag || m_drag->started || drag->origin() != m_window->surface())
        return;
    m_drag->started = true;
    if (m_drag->kind != DockIcon::Kind::Application)
        return;
    DockIcon *icon = applicationIcon(m_drag->key);
    drag->setPlainIdleCursor(true);
    // The icon is held at its centre, and goes back to its slot.
    drag->setReturnsHome(true);
    if (icon)
        drag->setHome(iconCentre(icon));
    m_applications->beginDrag(m_dock->applications().indexOf(m_drag->key));
    if (!m_drop.anchored)
        anchorApplicationArea();
}

void DockWindow::onDragMoved(const base::PointF &location)
{
    if (!m_drag || !m_drag->started || m_drag->kind != DockIcon::Kind::Application)
        return;
    const DockModel &model = m_dock->applications();
    const int row = model.indexOf(m_drag->key);
    if (row < 0 || !model.at(row).isPinned || model.at(row).fixed)
        return;
    // It undims nearer than it dims, so it does not flicker at the line.
    const double distance = distanceOffDock(location);
    const double thickness = m_metrics.thickness();
    if (distance > thickness * 1.5)
        m_drag->removes = true;
    else if (distance < thickness * 1.25)
        m_drag->removes = false;
    input::DragController *drag = m_shell->seat()->drag();
    drag->setIconOpacity(m_drag->removes ? 0.5f : 1.0f);
    drag->setReturnsHome(!m_drag->removes);
}

void DockWindow::onDragEnded()
{
    input::DragController *drag = m_shell->seat()->drag();
    const bool own = m_drag && m_drag->started && !m_drag->returning;
    if (own && drag->isReturning()) {
        // Nothing took it: the slot opens again under the icon on its way back.
        m_drag->removes = false;
        m_drag->returning = true;
        const DockModel &model = m_dock->applications();
        const int row = model.indexOf(m_drag->key);
        m_applications->setDropSlot(m_applications->draggedRow(), row >= 0 && model.at(row).isPinned);
    } else if (own) {
        finishIconDrag();
    } else if (!m_drag && m_applications->isDragging()) {
        // The launcher's drag ended without a drop on the dock.
        m_applications->endDrag();
    }
    resetDropTarget();
}

void DockWindow::finishIconDrag()
{
    if (!m_drag)
        return;
    const IconDrag drag = *m_drag;
    m_drag.reset();
    DockIcon *icon = drag.kind == DockIcon::Kind::Application ? applicationIcon(drag.key)
                                                              : minimizedIcon(drag.window);
    if (icon)
        icon->setDragHidden(false);
    if (drag.kind != DockIcon::Kind::Application)
        return;
    m_applications->endDrag();
    // Last: an icon of an application that is not running goes with its pin.
    if (drag.removeOnDrop)
        m_dock->applications().unPin(drag.key);
}

void DockWindow::resetDropTarget()
{
    setDropHovered(nullptr);
    const uint64_t generation = m_drop.generation + 1;
    m_drop = DropTarget();
    m_drop.generation = generation;
}

void DockWindow::dropEnter(ui::DropOffer *offer, const ui::PointF &position)
{
    // The same drag may come back in a new offer: what it carries is kept
    // until the drag ends.
    ++m_drop.generation;
    m_drop.offer = offer;
    m_drop.application = offer->hasMimeType(kApplicationMime);
    m_drop.desktop = offer->hasMimeType(kDesktopMime);
    m_drop.window = offer->hasMimeType(kWindowMime);
    m_drop.files = offer->hasMimeType("text/uri-list");
    const auto readId = [this, offer](const char *mime, bool desktop) {
        offer->read(mime, [this, alive = m_alive, generation = m_drop.generation, desktop](std::string data) {
            if (!*alive || generation != m_drop.generation)
                return;
            (desktop ? m_drop.desktopId : m_drop.appId) = std::string(base::strings::trimmed(data));
            if (m_drop.inArea) {
                enterApplicationArea();
                updateDropSlot();
            }
        });
    };
    if (m_drop.application && m_drag && m_drag->kind == DockIcon::Kind::Application)
        m_drop.appId = m_drag->key;
    else if (m_drop.application)
        readId(kApplicationMime, false);
    if (m_drop.desktop)
        readId(kDesktopMime, true);
    dropMove(position);
}

uint32_t DockWindow::dropMove(const ui::PointF &position)
{
    m_drop.position = position;
    if (m_drop.application || m_drop.desktop) {
        const bool inside = inApplicationArea(position);
        if (inside && !m_drop.inArea) {
            m_drop.inArea = true;
            enterApplicationArea();
        } else if (!inside && m_drop.inArea) {
            m_drop.inArea = false;
            m_applications->setDropSlot(-1, false);
            m_drop.slot = -1;
            m_drop.hasLastAlong = false;
        }
        if (m_drop.inArea)
            updateDropSlot();
    }
    setDropHovered(iconAt(position));

    if (m_drop.inArea && m_drop.slot >= 0)
        return ui::MoveAction;
    if (m_drop.hovered == m_trash && m_drop.files && m_drop.offer)
        return (m_drop.offer->sourceActions() & ui::MoveAction) ? ui::MoveAction : ui::CopyAction;
    if (m_drop.window && m_drop.hovered && m_drop.hovered->kind() == DockIcon::Kind::Minimized)
        return ui::MoveAction;
    return ui::NoAction;
}

void DockWindow::dropLeave()
{
    setDropHovered(nullptr);
    if (m_drop.inArea) {
        m_drop.inArea = false;
        m_applications->setDropSlot(-1, false);
        m_drop.slot = -1;
        m_drop.hasLastAlong = false;
    }
    m_drop.offer = nullptr;
}

uint32_t DockWindow::dropAt(const ui::PointF &position)
{
    m_drop.position = position;
    uint32_t action = ui::NoAction;
    if (m_drop.inArea && m_drop.entered) {
        m_drop.hasLastAlong = false;
        updateDropSlot();
        if (m_drop.slot >= 0) {
            DockModel &model = m_dock->applications();
            const int row = m_applications->sourceRowForSlot(m_drop.slot);
            if (m_drop.desktop) {
                if (m_drop.pinned && m_drop.desktopId) {
                    if (model.insert(*m_drop.desktopId, row, m_applications->draggedRow()))
                        action = ui::MoveAction;
                } else if (m_drop.pinned && m_drop.offer) {
                    // Dropped before its desktop id was read: pinned once it is.
                    m_drop.offer->read(kDesktopMime, [this, alive = m_alive, row,
                                                      existing = m_applications->draggedRow()](std::string data) {
                        const std::string id(base::strings::trimmed(data));
                        if (*alive && !id.empty())
                            m_dock->applications().insert(id, row, existing);
                    });
                    action = ui::MoveAction;
                }
            } else if (m_drop.appId && model.dropApplication(*m_drop.appId, row, m_drop.pinned)) {
                action = ui::MoveAction;
            }
        }
        m_applications->endDrag();
        m_drop.entered = false;
        m_drop.anchored = false;
        m_drop.slot = -1;
    } else if (m_drop.hovered == m_trash && m_drop.files && m_drop.offer) {
        action = (m_drop.offer->sourceActions() & ui::MoveAction) ? ui::MoveAction : ui::CopyAction;
        m_drop.offer->read("text/uri-list", [bus = m_shell->bus()](std::string data) {
            std::vector<std::string> paths;
            for (const std::string &line : base::strings::split(data, '\n')) {
                const std::string trimmed(base::strings::trimmed(line));
                if (trimmed.empty() || trimmed.starts_with('#'))
                    continue;
                if (const auto path = base::url::toLocalFile(trimmed))
                    paths.push_back(*path);
            }
            if (!paths.empty() && bus)
                TrashMonitor::moveToTrash(paths, bus);
        });
    } else if (m_drop.window && m_drop.hovered && m_drop.hovered->kind() == DockIcon::Kind::Minimized) {
        if (m_reorderTimer.isActive()) {
            m_reorderTimer.stop();
            reorderMinimized();
        }
        action = ui::MoveAction;
    }
    setDropHovered(nullptr);
    m_drop.offer = nullptr;
    return action;
}

void DockWindow::setDropHovered(DockIcon *icon)
{
    if (icon == m_drop.hovered)
        return;
    if (m_drop.hovered == m_trash)
        m_trash->setDropHover(false);
    m_raiseTimer.stop();
    m_reorderTimer.stop();
    m_drop.hovered = icon;
    if (!icon)
        return;
    switch (icon->kind()) {
    case DockIcon::Kind::Trash:
        // Any drag over it, as its DropArea took any.
        icon->setDropHover(true);
        break;
    case DockIcon::Kind::Application: {
        // Held over a running application, a dragged file raises its window.
        const DockModel &model = m_dock->applications();
        const int row = model.indexOf(icon->key());
        if (row >= 0 && !model.at(row).fixed && !m_drop.application && !m_drop.window)
            m_raiseTimer.start();
        break;
    }
    case DockIcon::Kind::Minimized:
        if (m_drop.window && m_drag && m_drag->window != icon->windowId())
            m_reorderTimer.start();
        break;
    }
}

void DockWindow::reorderMinimized()
{
    if (!m_drag || !m_drop.hovered || m_drop.hovered->kind() != DockIcon::Kind::Minimized)
        return;
    MinimizedWindows &minimized = m_dock->minimizedWindows();
    int from = -1;
    int to = -1;
    for (int row = 0; row < minimized.count(); ++row) {
        const uint64_t id = minimized.entries()[size_t(row)].id;
        if (id == m_drag->window)
            from = row;
        if (id == m_drop.hovered->windowId())
            to = row;
    }
    if (from >= 0 && to >= 0 && from != to)
        minimized.move(from, to);
}

DockIcon *DockWindow::iconAt(const ui::PointF &position) const
{
    for (DockIcon *icon : icons()) {
        if (!icon->isVisible() || !icon->isEnabled())
            continue;
        const ui::RectF rect = icon->geometry();
        if (position.x >= rect.x && position.x < rect.x + rect.width && position.y >= rect.y
            && position.y < rect.y + rect.height)
            return icon;
    }
    return nullptr;
}

bool DockWindow::inApplicationArea(const ui::PointF &position) const
{
    // From the panel's start through the applications and half a divider,
    // across the whole window.
    const bool bottom = m_dock->settings().direction() == DockSettings::Bottom;
    const double along = bottom ? position.x : position.y;
    const double origin = m_start + m_drawn.lengthPadding;
    return along >= origin - m_drawn.lengthPadding
            && along < origin + m_applications->length() + m_drawn.separatorExtent / 2;
}

void DockWindow::anchorApplicationArea()
{
    // Where position 0 is drawn, compensating for the length still to come
    // as the centred panel grows or shrinks.
    m_drop.referenceOrigin = m_start + m_drawn.lengthPadding
            - (m_applications->targetLength() + m_applications->targetTrailingExtent() - m_applications->length()
               - m_applications->trailingExtent())
                    / 2;
    m_drop.anchored = true;
    m_drop.hasLastAlong = false;
}

void DockWindow::enterApplicationArea()
{
    // What the drag carries must be known first; the launcher's is read.
    if ((m_drop.application && !m_drop.appId) || (m_drop.desktop && !m_drop.desktopId))
        return;
    const std::string appId = m_drop.appId.value_or(std::string());
    const std::string desktopId = m_drop.desktopId.value_or(std::string());
    if (!(m_drop.entered && m_applications->isDragging())) {
        if (m_drop.entered)
            m_applications->endDrag();
        if (!m_drop.anchored)
            anchorApplicationArea();
        DockModel &model = m_dock->applications();
        const int source = !desktopId.empty() ? model.desktopRow(desktopId) : model.indexOf(appId);
        m_drop.sourcePinned = source >= 0 && source < model.pinnedCount();
        m_applications->beginDrag(source, desktopId.empty());
    }
    m_drop.entered = true;
    m_drop.hasLastAlong = false;
}

void DockWindow::updateDropSlot()
{
    if (!m_drop.entered)
        return;
    const bool bottom = m_dock->settings().direction() == DockSettings::Bottom;
    const double along = (bottom ? m_drop.position.x : m_drop.position.y) - m_drop.referenceOrigin;
    // Layout animation must not change the slot under a stationary pointer.
    if (m_drop.hasLastAlong && std::abs(along - m_drop.lastAlong) < 0.5)
        return;
    m_drop.lastAlong = along;
    m_drop.hasLastAlong = true;

    const DockModel &model = m_dock->applications();
    const double pitch = m_drawn.iconSize + m_drawn.iconGap;
    const double iconGap = m_drawn.iconGap;
    const double separatorExtent = m_drawn.separatorExtent;
    const bool external = m_drop.desktopId && !m_drop.desktopId->empty();
    const int source = m_applications->removedRow();
    const int remaining = model.count() - (source >= 0 ? 1 : 0);
    const int pins = model.pinnedCount() - (source >= 0 && source < model.pinnedCount() ? 1 : 0);
    const int pinGap = m_applications->placeholder() >= 0 && m_applications->placeholderPinned() ? 1 : 0;
    const double externalShift = external ? pitch / 2 : 0;
    // The reference was anchored with the dragged icon's own slot open, or none
    // for a drag from elsewhere; the centred dock is drawn half a slot along
    // for each slot fewer open now.
    const int anchoredSlots = source >= 0 ? 1 : 0;
    const int openSlots = m_applications->placeholder() >= 0 ? 1 : 0;
    const double drawnShift = (anchoredSlots - openSlots) * pitch / 2;
    // One that can only be pinned takes the whole divider gap; one that can
    // go either way changes sides at the line.
    const bool pinsOnly = m_drop.sourcePinned || external;
    const double divider = pinsOnly ? separatorExtent : (separatorExtent - iconGap) / 2;
    const double boundary = (pins + pinGap) * pitch + divider + drawnShift;
    m_drop.pinned = along < boundary;
    if (!m_drop.pinned && pinsOnly) {
        m_drop.slot = -1;
        m_applications->setDropSlot(-1, false);
        return;
    }
    const int row = int(std::floor((along + (m_drop.pinned ? externalShift : -separatorExtent) + iconGap / 2) / pitch));
    m_drop.slot = m_drop.pinned ? std::max(model.firstMovableRow(), std::min(pins, row))
                                : std::max(pins, std::min(remaining, row));
    m_applications->setDropSlot(m_drop.slot, m_drop.pinned);
}

} // namespace Lingmo::Shell
