// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/launcherwidget.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "base/desktop_entry.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/process.h"
#include "base/dbus.h"
#include "base/settings.h"
#include "core/config.h"
#include "ui/color_id.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/theme.h"

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

// ── AppIcon ──────────────────────────────────────────────────────────────────

class LauncherWidget::AppIcon : public ui::View
{
public:
    AppIcon(LauncherWidget::App app, int iconSize)
        : m_app(std::move(app))
        , m_iconSize(iconSize)
    {
        setCursorShape(ui::CursorShape::Pointer);
        m_icon = addChild<ui::ThemeIcon>(m_app.icon, double(iconSize));
        m_label = addChild<ui::Label>();
        m_label->setText(m_app.name);
        m_label->setTextAlignment(ui::Label::TextAlignment::Center);
        m_label->setElide(ui::Label::Elide::Right);
        m_label->setFontSize(10);
        m_label->setWrap(true);
        m_label->setMaximumLineCount(2);
        setToolTip(m_app.name);
    }

    bool acceptsPointer() const override { return true; }

    void paint(ui::Painter& painter) override
    {
        if (m_hovered) {
            const ui::Theme& theme = widgetTheme(this);
            painter.fillRoundedRect(localRect(), 8, theme.text.withAlpha(0.09));
        }
    }

    void layout() override
    {
        const ui::RectF cell = localRect();
        const double iconAreaH = cell.height * 0.70;
        m_iconSize=int(std::max(12.0,std::min({double(m_iconSize),cell.width-12,iconAreaH-6})));
        m_icon->setSize(m_iconSize);
        m_icon->setColor(widgetMonochrome(this) ? std::optional<ui::ColorRecipe>(widgetTheme(this).text) : std::nullopt);
        const double iconX = cell.x + (cell.width - m_iconSize) / 2.0;
        const double iconY = cell.y + (iconAreaH - m_iconSize) / 2.0;
        m_icon->setGeometry({iconX, iconY, double(m_iconSize), double(m_iconSize)});
        const double labelY = cell.y + iconAreaH;
        m_label->setGeometry({cell.x + 2, labelY, std::max(0.0, cell.width - 4),
                               std::max(0.0, cell.height - iconAreaH)});
    }

protected:
    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        if (!localRect().contains(event.position))
            return;
        if (m_app.command.empty())
            return;
        const std::string program = m_app.command[0];
        const std::vector<std::string> args(m_app.command.begin() + 1,
                                            m_app.command.end());
        if(auto* bus=base::dbus::Connection::session())
            base::dbus::Proxy(bus,"org.lingmo.AppRuntime","/AppRuntime","org.lingmo.AppRuntime")
                .send("launchCommand",m_app.desktopId,m_app.command,std::string(),std::string());
    }

    void hoverChanged() override { requestPaint(); }

private:
    LauncherWidget::App m_app;
    int m_iconSize;
    bool m_hovered = false;
    ui::ThemeIcon* m_icon = nullptr;
    ui::Label* m_label = nullptr;
};

// ── LauncherWidget ────────────────────────────────────────────────────────────

LauncherWidget::LauncherWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::Launcher, size)
{
    setName("launcher-widget");
    loadPins();
    rebuildIcons();
}

// ── helpers ───────────────────────────────────────────────────────────────────

std::string LauncherWidget::settingsFile()
{
    return fm::settingsFile();
}

int LauncherWidget::columns() const
{
    return (size() == WidgetSize::Small) ? 2 : 4;
}

int LauncherWidget::maxSlots() const
{
    switch (size()) {
    case WidgetSize::Small:  return 4;
    case WidgetSize::Medium: return 8;
    case WidgetSize::Large:  return 16;
    }
    return 8;
}

// ── loadPins ──────────────────────────────────────────────────────────────────

void LauncherWidget::loadPins()
{
    m_apps.clear();

    base::Settings settings(settingsFile());
    const std::string pins = settings.stringValue("Launcher/Pins");

    // Split the comma-separated list of desktop IDs
    std::vector<std::string> ids;
    if (!pins.empty()) {
        std::istringstream ss(pins);
        std::string token;
        while (std::getline(ss, token, ',')) {
            // Trim whitespace
            const auto first = token.find_first_not_of(" \t");
            const auto last  = token.find_last_not_of(" \t");
            if (first != std::string::npos)
                ids.push_back(token.substr(first, last - first + 1));
        }
    }

    // Resolve each ID to a DesktopEntry
    const std::string localApps =
            base::paths::dataHome() + "/applications/";
    const std::string systemApps = "/usr/share/applications/";

    for (const std::string& id : ids) {
        // Try user-local first, then system
        for (const std::string& dir : {localApps, systemApps}) {
            const std::string path = dir + id + (id.ends_with(".desktop") ? "" : ".desktop");
            if (!std::filesystem::exists(path))
                continue;
            // Read file contents for DesktopEntry::parse
            std::ifstream file(path);
            if (!file)
                continue;
            const std::string contents((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
            const auto entry = base::DesktopEntry::parse(id, path, contents);
            if (!entry)
                continue;
            const std::vector<std::string> cmd = entry->command();
            if (cmd.empty())
                continue;
            App app;
            app.desktopId = id;
            app.name      = entry->name;
            app.icon      = entry->icon;
            app.command   = cmd;
            m_apps.push_back(std::move(app));
            break;
        }
        if (static_cast<int>(m_apps.size()) >= maxSlots())
            break;
    }

    // Fallback: scan /usr/share/applications/ if still empty
    if (m_apps.empty()) {
        std::error_code scanError;
        for (const auto& dirEntry : std::filesystem::directory_iterator(systemApps,scanError)) {
            if (dirEntry.path().extension() != ".desktop")
                continue;
            const std::string path = dirEntry.path().string();
            const std::string id   = dirEntry.path().stem().string();
            std::ifstream file(path);
            if (!file)
                continue;
            const std::string contents((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
            const auto entry = base::DesktopEntry::parse(id, path, contents);
            if (!entry || !entry->shouldShow())
                continue;
            const std::vector<std::string> cmd = entry->command();
            if (cmd.empty())
                continue;
            App app;
            app.desktopId = id;
            app.name      = entry->name;
            app.icon      = entry->icon;
            app.command   = cmd;
            m_apps.push_back(std::move(app));
            if (static_cast<int>(m_apps.size()) >= maxSlots())
                break;
        }
    }
}

// ── rebuildIcons ──────────────────────────────────────────────────────────────

void LauncherWidget::rebuildIcons()
{
    for (AppIcon* icon : m_icons)
        takeChild(icon);
    m_icons.clear();

    const int slots = maxSlots();
    const int iconSize = (size() == WidgetSize::Small) ? 36 : 48;

    const int count = std::min(static_cast<int>(m_apps.size()), slots);
    for (int i = 0; i < count; ++i) {
        AppIcon* icon = addChild<AppIcon>(m_apps[i], iconSize);
        m_icons.push_back(icon);
    }
    requestLayout();
}

// ── layoutContent ─────────────────────────────────────────────────────────────

void LauncherWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    if (m_icons.empty())
        return;

    const int cols = columns();
    const int rows = (static_cast<int>(m_icons.size()) + cols - 1) / cols;

    const double cellW = content.width  / cols;
    const double cellH = (rows > 0) ? content.height / rows : content.height;

    for (int i = 0; i < static_cast<int>(m_icons.size()); ++i) {
        const int col = i % cols;
        const int row = i / cols;
        m_icons[i]->setGeometry({
            content.x + col * cellW,
            content.y + row * cellH,
            cellW,
            cellH,
        });
    }
}

// ── retranslateContent ────────────────────────────────────────────────────────

void LauncherWidget::retranslateContent()
{
    loadPins();
    rebuildIcons();
}

} // namespace fm
