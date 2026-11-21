// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "lingmo/networkmanager/network_model.h"

namespace Lingmo::Networkmanager {
class ConnectionSettings;
class Handler;
}

namespace Lingmo::ui {
class ColumnLayout;
class DialogButton;
class DialogWindow;
}

namespace settings {
class InfoRow;
class SettingsWindow;
class SwitchRow;
}

namespace settings::network {

class IpSettingsView;
class NetworkList;

// A Wi-Fi network's or a wired profile's details: whether it connects by
// itself, its addresses, and the way to its IP settings, which take the
// details' place with a way back in the title bar. The network is followed
// across the list's changes; the dialog closes once it is gone.
class DetailsDialog
{
public:
    enum class Kind {
        Wireless,
        Wired,
    };

    // A saved network is followed by its connection, others by name.
    DetailsDialog(SettingsWindow& window, NetworkList& list, Kind kind,
                  const Lingmo::Networkmanager::NetworkItem& item);
    ~DetailsDialog();

    DetailsDialog(const DetailsDialog&) = delete;
    DetailsDialog& operator=(const DetailsDialog&) = delete;

    void show();
    Lingmo::ui::DialogWindow* dialog() const { return m_dialog.get(); }

    std::function<void()> onClosed;

private:
    class Content;

    std::optional<Lingmo::Networkmanager::NetworkItem> find() const;
    void update();
    void showIpSettings(bool shown);
    void setSaved(bool saved);
    void close();

    SettingsWindow& m_window;
    NetworkList& m_list;
    Kind m_kind;
    std::string m_connectionPath;
    std::string m_ssid;
    std::unique_ptr<Lingmo::Networkmanager::Handler> m_handler;
    std::unique_ptr<Lingmo::Networkmanager::ConnectionSettings> m_settings;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_dialog;
    Lingmo::base::Subscription m_subscription;
    Lingmo::Networkmanager::NetworkItem m_row;
    bool m_found = false;
    bool m_saved = false;
    bool m_ipSettingsShown = false;

    Content* m_content = nullptr;
    Lingmo::ui::ColumnLayout* m_details = nullptr;
    Lingmo::ui::ColumnLayout* m_savedCard = nullptr;
    SwitchRow* m_autoConnect = nullptr;
    InfoRow* m_security = nullptr;
    InfoRow* m_signal = nullptr;
    Lingmo::ui::ColumnLayout* m_addressCard = nullptr;
    InfoRow* m_ipv4 = nullptr;
    InfoRow* m_ipv6 = nullptr;
    InfoRow* m_mac = nullptr;
    InfoRow* m_gateway = nullptr;
    InfoRow* m_dns = nullptr;
    Lingmo::ui::DialogButton* m_forget = nullptr;
    IpSettingsView* m_ipSettings = nullptr;
};

} // namespace settings::network
