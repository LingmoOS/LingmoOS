// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/detailsdialog.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "pages/network/common/ipsettingsview.h"
#include "pages/network/common/networklist.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/cell_layout.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

static constexpr double kWidth = 400;

// The dialog's content below its title: the details, or the IP settings in
// their place. Escape leaves the IP settings before it closes the dialog.
class DetailsDialog::Content : public ColumnLayout
{
public:
    std::function<bool()> onEscape;

protected:
    bool keyPress(const KeyEvent& event) override
    {
        if (event.key == key::Escape && event.modifiers == NoModifier && onEscape)
            return onEscape();
        return false;
    }
};

DetailsDialog::DetailsDialog(SettingsWindow& window, NetworkList& list, Kind kind, const nm::NetworkItem& item)
    : m_window(window)
    , m_list(list)
    , m_kind(kind)
    , m_connectionPath(item.connectionPath)
    , m_ssid(item.connectionPath.empty() ? item.ssid : std::string())
    , m_handler(std::make_unique<nm::Handler>())
    , m_dialog(std::make_unique<DialogWindow>(&window.window(), item.uniqueName, kWidth))
{
    auto content = std::make_unique<Content>();
    m_content = content.get();
    m_content->setPadding({0, metrics::dialogPadding, 0, 0});
    m_content->onEscape = [this] {
        if (m_ipSettingsShown)
            showIpSettings(false);
        else
            close();
        return true;
    };

    m_details = m_content->addChild<ColumnLayout>();
    m_details->setSpacing(metrics::dialogSpacing);
    m_details->setLayoutHints({.fillWidth = true});

    m_savedCard = m_details->addChild<Card>(Card::Style::List);
    m_autoConnect =
            m_savedCard->addChild<SwitchRow>(kind == Kind::Wireless ? tr("Auto-Join") : tr("Connect Automatically"));
    m_autoConnect->onToggled = [this](bool checked) {
        if (!m_list.isMock() && m_settings)
            m_settings->setAutoConnect(checked);
    };
    NavigationItem* ipSettings = m_savedCard->addChild<NavigationItem>(tr("IP Settings"));
    ipSettings->setEnabled(!m_list.isMock());
    ipSettings->onClicked = [this] { showIpSettings(true); };

    m_addressCard = m_details->addChild<Card>(Card::Style::List);
    if (kind == Kind::Wireless) {
        m_security = m_addressCard->addChild<InfoRow>(tr("Security"));
        m_signal = m_addressCard->addChild<InfoRow>(tr("Signal"));
    }
    m_ipv4 = m_addressCard->addChild<InfoRow>(tr("IPv4 address"));
    m_ipv6 = m_addressCard->addChild<InfoRow>(tr("IPv6 address"));
    m_mac = m_addressCard->addChild<InfoRow>(tr("MAC address"));
    m_gateway = m_addressCard->addChild<InfoRow>(tr("Gateway"));
    m_dns = m_addressCard->addChild<InfoRow>(tr("DNS"));

    RowLayout* buttons = m_details->addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    DialogButton* closeButton = buttons->addChild<DialogButton>(tr("Close"));
    closeButton->onClicked = [this] { close(); };
    if (kind == Kind::Wireless) {
        m_forget = buttons->addChild<DialogButton>(tr("Forget this network"), true);
        m_forget->setDestructive(true);
        m_forget->onClicked = [this] {
            if (!m_list.isMock())
                m_handler->removeConnection(m_row.connectionPath);
            close();
        };
    }

    m_dialog->setContentView(std::move(content));
    m_dialog->onBack = [this] { showIpSettings(false); };
    m_dialog->onClosed = [this] {
        m_subscription.reset();
        if (onClosed)
            onClosed();
    };
    m_dialog->window()->addShortcut(NoModifier, key::Escape, [this] { m_content->onEscape(); });

    // The wired dialog knows its profile from the start; Wi-Fi only once it is saved.
    if (kind == Kind::Wired) {
        if (!m_list.isMock())
            m_settings = std::make_unique<nm::ConnectionSettings>(m_connectionPath);
        m_autoConnect->setChecked(m_list.isMock() || (m_settings && m_settings->autoConnect()));
    }

    m_subscription = m_list.changed.connect([this] { update(); });
    update();
}

DetailsDialog::~DetailsDialog() = default;

void DetailsDialog::show()
{
    m_dialog->show();
}

std::optional<nm::NetworkItem> DetailsDialog::find() const
{
    // In the list's order, as the first row with that connection or name.
    for (const nm::NetworkItem& item :
         m_list.rows(m_kind == Kind::Wireless ? nm::ConnectionType::Wireless : nm::ConnectionType::Wired)) {
        if (!m_connectionPath.empty() ? item.connectionPath == m_connectionPath : item.ssid == m_ssid)
            return item;
    }
    return std::nullopt;
}

void DetailsDialog::update()
{
    const std::optional<nm::NetworkItem> row = find();
    if (!row) {
        if (m_found)
            close();
        return;
    }
    m_found = true;
    m_row = *row;

    if (m_kind == Kind::Wireless) {
        setSaved(!m_row.connectionPath.empty());
        m_security->setValue(m_row.securityTypeString);
        // TRANSLATORS: a percentage, such as "92%".
        m_signal->setValue(format(tr("{percent}%"), {{"percent", std::to_string(m_row.signal)}}));
    } else {
        // Only a connection that is up has addresses.
        m_addressCard->setVisible(!m_row.ipV4Address.empty() || !m_row.ipV6Address.empty());
    }
    m_ipv4->setValue(m_row.ipV4Address);
    m_ipv6->setValue(m_row.ipV6Address);
    m_mac->setValue(m_row.macAddress);
    m_gateway->setValue(m_row.gateway);
    m_dns->setValue(m_row.nameServer);
    if (!m_ipSettingsShown)
        m_dialog->setTitle(m_row.uniqueName);
}

// Only a saved network has settings to auto-join with or forget.
void DetailsDialog::setSaved(bool saved)
{
    m_savedCard->setVisible(saved);
    m_forget->setVisible(saved);
    if (saved == m_saved)
        return;
    m_saved = saved;
    if (!saved)
        return;
    // The sample networks have no NetworkManager connection behind them.
    if (!m_list.isMock())
        m_settings = std::make_unique<nm::ConnectionSettings>(m_row.connectionPath);
    m_autoConnect->setChecked(m_list.isMock() || (m_settings && m_settings->autoConnect()));
}

void DetailsDialog::showIpSettings(bool shown)
{
    if (shown == m_ipSettingsShown)
        return;
    m_ipSettingsShown = shown;
    m_details->setVisible(!shown);
    if (shown) {
        // Made each time it is shown, so it starts from the saved profile.
        m_ipSettings = m_content->addChild<IpSettingsView>(m_row.connectionPath);
        m_ipSettings->onFinished = [this] { showIpSettings(false); };
    } else if (m_ipSettings) {
        // Not destroyed from inside its own callback.
        std::shared_ptr<View> gone = m_content->takeChild(m_ipSettings);
        m_ipSettings = nullptr;
        Lingmo::base::EventLoop::current()->post([gone] {});
    }
    m_dialog->setBackVisible(shown);
    m_dialog->setTitle(shown ? std::string(tr("IP Settings")) : m_row.uniqueName);
}

void DetailsDialog::close()
{
    if (!m_dialog->window())
        return;
    m_dialog->close();
}

} // namespace settings::network
