// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowicon.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

namespace {

// Network services that aren't a device of their own, each on a page of its own.
class NetworkPage : public Page
{
public:
    explicit NetworkPage(SettingsWindow& window)
        : Page(window, tr("Network"))
    {
        Card* card = content()->addChild<Card>(Card::Style::List);
        NavigationItem* vpn = card->addChild<NavigationItem>(tr("VPN"));
        vpn->setName("network:vpn");
        vpn->setLeading<RowIcon>("network-vpn", Color::rgb(0x48484C));
        vpn->onClicked = [this] { settingsWindow().pushPage("vpn"); };

        NavigationItem* proxy = card->addChild<NavigationItem>(tr("Proxy"));
        proxy->setName("network:proxy");
        proxy->setLeading<RowIcon>("proxy.svg", Color::rgb(0x0067FF));
        proxy->onClicked = [this] { settingsWindow().pushPage("proxy"); };

        m_hotspot = card->addChild<NavigationItem>(tr("Hotspot"));
        m_hotspot->setName("network:hotspot");
        m_hotspot->setLeading<RowIcon>("network-wireless-hotspot", Color::rgb(0x2EBC6B));
        m_hotspot->onClicked = [this] { settingsWindow().pushPage("hotspot"); };

        m_subscriptions.push_back(m_enabled.changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_handler.hotspotSupportedChanged.connect([this] { update(); }));
        update();
    }

private:
    void update()
    {
        // Shown until NetworkManager has been read, so the row doesn't pop in late.
        m_hotspot->setVisible(!m_enabled.isReady() || m_handler.hotspotCapable());
    }

    nm::EnabledConnections m_enabled;
    nm::Handler m_handler;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    NavigationItem* m_hotspot;
};

} // namespace

static const PageRegistration registration("network", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<NetworkPage>(window);
});

} // namespace settings::network
