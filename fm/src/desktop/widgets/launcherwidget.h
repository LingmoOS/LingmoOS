// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>
#include <vector>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class Label;
}

namespace fm {

// A grid of pinned application launchers. Tapping an icon starts the app.
// The pin list is a comma-separated list of desktop IDs stored under
// Launcher/Pins in the widget's settings file; it falls back to a scan of
// /usr/share/applications when the list is empty or unparseable.
class LauncherWidget : public DesktopWidget
{
public:
    LauncherWidget(WidgetLayer& layer, WidgetSize size);
    ~LauncherWidget() override = default;

protected:
    void layoutContent() override;
    void retranslateContent() override;

private:
    struct App {
        std::string desktopId;
        std::string name;
        std::string icon;
        std::vector<std::string> command;
    };

    class AppIcon;

    void loadPins();
    void rebuildIcons();
    int columns() const;   // 2 for Small, 4 for Medium/Large
    int maxSlots() const;  // 4 for Small, 8 for Medium, 16 for Large

    static std::string settingsFile();

    std::vector<App> m_apps;
    std::vector<AppIcon*> m_icons;
};

} // namespace fm
