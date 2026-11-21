// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "model/foldermodel.h"

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"
#include "ui/geometry.h"

#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
struct KeyEvent;
class Menu;
class TrustedConnection;
class Window;
}

namespace fm {

class DesktopGridView;
class DesktopMenuExporter;
class DesktopRoot;
class FileManagerHost;
class PreviewClient;
class WidgetGallery;
class WidgetLayer;

// The desktop: the icons of ~/Desktop in a transparent window with the
// desktop role on the primary output, over the compositor's wallpaper, on
// the session's trusted connection.
class DesktopWindow : public FolderModelHost
{
public:
    // Without a connection, an ordinary window of the output's size, for
    // trying the desktop out in a session that has one already.
    DesktopWindow(ui::TrustedConnection* connection, FileManagerHost& host);
    ~DesktopWindow() override;

    ui::Window* window() const { return m_window.get(); }
    DesktopGridView* view() const { return m_view; }
    WidgetLayer* widgets() const { return m_widgets; }
    // Opens the widget gallery on the desktop, as the desktop's own menu does.
    void editWidgets();
    void editNotificationWidgets();
    // Reads the widget settings again, such as the weather city.
    void refreshWidgets();
    void refreshWidgetAppearance();
    FolderModel& model() { return *m_model; }

    // FolderModelHost.
    void openFolders(const std::vector<std::string>& paths) override;
    void showProperties(const FileItemList& items) override;
    void showOpenWith(const std::string& url) override;
    void confirmDelete(const std::vector<std::string>& urls) override;
    void confirmEmptyTrash() override;
    void confirmLaunch(const std::string& path) override;
    void notify(const std::string& message) override;

private:
    bool m_notificationTarget = false;
    friend class DesktopRoot;

    void followPrimaryOutput();
    void placeOn(const std::string& outputName);
    void retranslate();
    void refreshDesktop();
    void showContextMenu(ui::PointF position);
    bool handleKey(const ui::KeyEvent& event);
    void loadLayout();
    void saveLayout();
    std::string layoutKey() const;
    void loadWidgetLayout();
    void saveWidgetLayout();
    std::string widgetLayoutKey() const;
    void openWidgetGallery();
    void closeWidgetGallery();
    // Desktop view / sort options.
    bool desktopIconsVisible() const { return m_showDesktopIcons; }
    void setDesktopIconsVisible(bool visible);
    bool autoArrange() const { return m_autoArrange; }
    void setAutoArrange(bool enabled);
    void alignIconsToGrid();

    ui::TrustedConnection* m_connection;
    FileManagerHost& m_host;
    std::unique_ptr<FolderModel> m_model;
    std::unique_ptr<ui::Window> m_window;
    DesktopRoot* m_root = nullptr;
    DesktopGridView* m_view = nullptr;
    WidgetLayer* m_widgets = nullptr;
    WidgetGallery* m_gallery = nullptr;
    std::unique_ptr<PreviewClient> m_preview;
    std::unique_ptr<ui::Menu> m_menu;
    std::unique_ptr<DesktopMenuExporter> m_menuExporter;
    std::unique_ptr<class ArchivePasswordPrompt> m_archivePassword;
    std::string m_output;
    bool m_layoutLoaded = false;
    bool m_restoring = false;
    bool m_activated = false;
    bool m_widgetsLoaded = false;
    bool m_showDesktopIcons = true;
    bool m_autoArrange = false;
    base::Timer m_saveLayout;
    base::Timer m_saveWidgets;
    base::Timer m_restoreLayout;
    base::Timer m_menuUpdate;
    base::dbus::Proxy m_display;
    base::dbus::Match m_displayChanged;
    base::dbus::PendingCall m_displayState;
    base::dbus::NameWatch m_displayWatch;
    std::vector<base::Subscription> m_subscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
