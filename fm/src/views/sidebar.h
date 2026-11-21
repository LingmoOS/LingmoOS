// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"
#include "ui/view.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class ConfirmDialog;
class Label;
class Menu;
class ScrollView;
}

namespace fm {

class ConnectServerDialog;
class PlacesModel;
class ServerPasswordDialog;
class SidebarList;
class UnlockVolumeDialog;

// The window's title over the places, favourites first, which
// take drops and reorder by dragging.
class SideBar : public ui::View
{
public:
    // Dragging its edge resizes it in this range.
    static constexpr double kDefaultWidth = 190;
    static constexpr double kMinimumWidth = 140;
    static constexpr double kMaximumWidth = 400;

    // The header's height, which the title fills.
    explicit SideBar(double headerHeight);
    ~SideBar() override;

    PlacesModel& places() const { return *m_places; }
    void setTitle(std::string title);
    // The places and the open dialogs in the language of now; what the
    // dialogs were given stays.
    void retranslate();
    // Lights the place showing the folder.
    void updateSelection(const std::string& path);
    // The Connect to Server dialog, over the window.
    void connectToServer();

    std::function<void(const std::string& path)> onClicked;
    std::function<void(const std::string& path)> onOpenInNewTab;
    std::function<void(const std::string& path)> onOpenInNewWindow;
    std::function<void(const std::string& message)> onNotification;

    ui::SizeF sizeHint() const override { return {kDefaultWidth, 0}; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;

private:
    friend class SidebarList;

    void askPassword(std::string address, std::string user, std::string domain, bool needsDomain,
                     bool anonymousAllowed, bool retry);
    void askHostKey(const std::string& address, const std::string& fingerprint, bool changed);
    void askPassphrase(std::string udi, std::string name, bool retry);
    void showDeviceError(const std::string& message);
    void removeConfirm(ui::ConfirmDialog* dialog);

    std::unique_ptr<PlacesModel> m_places;
    std::unique_ptr<ConnectServerDialog> m_connectDialog;
    std::vector<std::unique_ptr<ServerPasswordDialog>> m_passwordDialogs;
    std::vector<std::unique_ptr<UnlockVolumeDialog>> m_unlockDialogs;
    std::vector<std::unique_ptr<ui::ConfirmDialog>> m_confirmDialogs;
    // Each open question's title, text and button, set again as the language changes.
    std::map<ui::ConfirmDialog*, std::function<void()>> m_confirmTexts;
    double m_headerHeight;
    ui::Label* m_title;
    ui::ScrollView* m_scroll;
    SidebarList* m_list;
    std::string m_selectedPath;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
