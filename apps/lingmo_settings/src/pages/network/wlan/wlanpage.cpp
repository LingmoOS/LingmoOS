// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <chrono>
#include <optional>

#include "base/i18n.h"
#include "base/dbus.h"
#include "ui/button.h"
#include "base/timer.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "lingmo/networkmanager/hotspot.h"
#include "pages/network/common/detailsdialog.h"
#include "pages/network/common/networklist.h"
#include "pages/network/common/rows.h"
#include "pages/network/wlan/newnetworkdialog.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/options.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using namespace std::chrono_literals;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

namespace {

// A titled card of the networks a section takes; hidden while it has none.
struct WifiSection
{
    Section* section = nullptr;
    Card* card = nullptr;
    std::unique_ptr<KeyedRows<WifiRow>> rows;
    // A last row that is always there, for joining a hidden network.
    ListRow* footer = nullptr;
};

class WlanPage : public Page
{
public:
    explicit WlanPage(SettingsWindow& window)
        : Page(window, tr("Wi-Fi"))
        , m_newNetwork(window)
    {
        content()->setSpacing(metrics::sectionSpacing);
        m_newNetwork.onConnect = [this](const std::string& ssid, const std::string& userName,
                                        const std::string& password, const std::string& type) {
            if (!m_list.isMock())
                m_handler.addOtherConnection(ssid, userName, password, type);
        };
        m_newNetwork.onJoinNetwork = [this](const std::string& device, const std::string& accessPoint,
                                            const std::string& userName, const std::string& password) {
            if (!m_list.isMock())
                m_handler.addAndActivateEnterprise(device, accessPoint, userName, password);
        };

        // The switch and the network in use share the first card; the rest
        // is split into the networks with a saved connection and the others.
        m_view = content()->addChild<ColumnLayout>();
        m_view->setSpacing(metrics::sectionSpacing);
        m_view->setLayoutHints({.fillWidth = true});
        m_mainCard = m_view->addChild<Card>(Card::Style::List);
        m_switch = m_mainCard->addChild<SwitchRow>(tr("Wi-Fi"));
        m_switch->setName("wlan:switch");
        m_switch->onToggled = [this](bool checked) { wirelessToggled(checked); };
        m_currentRows = std::make_unique<KeyedRows<WifiRow>>([this](View* parent) { return makeRow(parent); });

        // The adapter serves the hotspot instead of joining networks, so there is nothing to list.
        m_hotspotRow = m_mainCard->addChild<ListRow>();
        m_hotspotRow->setName("wlan:hotspot");
        m_hotspotRow->setLeading<Icon>("network-wireless-hotspot", 18);
        m_hotspotLabel = m_hotspotRow->addContent<Label>(tr("Hotspot"));
        m_hotspotChevron = m_hotspotRow->addContent<TrailingIcon>("chevron-right", 16);
        m_hotspotRow->onClicked = [this] { settingsWindow().switchToPage("hotspot"); };

        m_connectivityCard = m_view->addChild<Card>();
        m_connectivityCard->setVisible(false);
        m_connectivityLabel = m_connectivityCard->addChild<Label>();
        m_connectivityLabel->setWrap(true);
        m_connectivityLabel->setLayoutHints({.fillWidth = true});
        m_login = m_connectivityCard->addChild<Button>(tr("Open login page"));
        m_login->setName("wlan:login");
        m_login->onClicked = [this] {
            m_loginCall = m_connectivity.callAsync("openLogin", [this](const Lingmo::base::dbus::Reply& reply) {
                if (!reply.value<bool>().value_or(false))
                    m_connectivityLabel->setText(tr("Could not open the network login page."));
            });
        };
        m_connectivityProperties = m_connectivity.onPropertiesChanged(
                [this](const Lingmo::base::dbus::VariantMap&, const std::vector<std::string>&) { refreshConnectivity(); });
        if (!m_list.isMock()) {
            if (auto* bus = Lingmo::base::dbus::Connection::session())
                m_connectivityOwner = bus->watchName("org.lingmo.Services", [this](const std::string& owner) {
                    ++m_connectivityGeneration;
                    m_connectivityRead = {};
                    m_connectivityCard->setVisible(false);
                    if (!owner.empty())
                        refreshConnectivity();
                });
        }

        m_known = addSection(tr("Known networks"), false);
        m_known.section->setName("wlan:known");
        m_other = addSection(tr("Other networks"), true);
        m_other.section->setName("wlan:other");
        m_other.footer->setName("wlan:addOther");

        m_subscriptions.push_back(m_list.changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_enabled.changed.connect([this] {
            settleWireless();
            update();
            joinRequestedNetwork();
        }));
        m_subscriptions.push_back(m_handler.hotspotActiveChanged.connect([this] { update(); }));
        m_subscriptions.push_back(
            nm::HotspotConfiguration::instance()->changed.connect([this] { update(); }));
        m_subscriptions.push_back(window.wifiJoinRequested.connect([this] { joinRequestedNetwork(); }));
        m_subscriptions.push_back(window.revealedChanged.connect([this] { joinRequestedNetwork(); }));

        if (!m_list.isMock()) {
            m_handler.requestScan();
            m_scanTimer.onTimeout = [this] { m_handler.requestScan(); };
            m_scanTimer.start(10200ms);
        }
        update();
        joinRequestedNetwork();
    }

protected:
    void layout() override
    {
        const Color disabled = Theme::current().disabledText;
        m_hotspotLabel->setColor(disabled);
        m_hotspotChevron->icon()->setColor(disabled);
        Page::layout();
    }

private:
    void refreshConnectivity()
    {
        const uint64_t generation = ++m_connectivityGeneration;
        m_connectivityRead = m_connectivity.getAllAsync([this, generation](const Lingmo::base::dbus::Reply& reply) {
            if (generation != m_connectivityGeneration)
                return;
            auto properties = reply.value<Lingmo::base::dbus::VariantMap>();
            const bool wireless = properties && (*properties)["wireless"].value<bool>();
            m_connectivityCard->setVisible(wireless && !m_list.isMock());
            if (!wireless)
                return;
            const uint32_t state = (*properties)["state"].value<uint32_t>();
            m_login->setVisible(state == 2);
            m_connectivityLabel->setText(state == 2 ? tr("Connected to Wi-Fi — login required")
                    : state == 4 ? tr("Internet access available")
                    : state == 1 || state == 3 ? tr("Connected to Wi-Fi — Internet access is limited")
                    : tr("Connected to Wi-Fi — Internet access has not been verified"));
        });
    }

    WifiSection addSection(const std::string& title, bool footer)
    {
        WifiSection section;
        section.section = m_view->addChild<Section>(title);
        section.section->setLayoutHints({.fillWidth = true});
        section.card = section.section->addChild<Card>(Card::Style::List);
        section.rows = std::make_unique<KeyedRows<WifiRow>>([this](View* parent) { return makeRow(parent); });
        if (footer) {
            section.footer = section.card->addChild<ListRow>(tr("Add other…"));
            // Lines the text up with the network names beside their icons.
            View* spacer = section.footer->setLeading<View>();
            spacer->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
            section.footer->onClicked = [this] { m_newNetwork.show(); };
        }
        return section;
    }

    WifiRow* makeRow(View* parent)
    {
        WifiRow* row = parent->addChild<WifiRow>();
        row->onClicked = [this, row] {
            if (row->displayedState() == nm::ConnectionStatus::Deactivated)
                connectNetwork(row->item());
        };
        row->onInfo = [this](WifiRow* row) { showDetails(row->item()); };
        return row;
    }

    void connectNetwork(const nm::NetworkItem& item)
    {
        if (!item.uuid.empty()) {
            if (!m_list.isMock())
                m_handler.activateConnection(item.connectionPath, item.devicePath, item.specificPath);
        } else if (isEnterprise(item.securityType)) {
            m_newNetwork.openNetwork(item.specificPath, m_list.isMock() ? std::nullopt
                                                                         : m_handler.wirelessNetwork(item.specificPath));
        } else if (!m_list.isMock()) {
            m_handler.addAndActivateConnection(item.devicePath, item.specificPath);
        }
    }

    void showDetails(const nm::NetworkItem& item)
    {
        m_details = std::make_unique<DetailsDialog>(settingsWindow(), m_list, DetailsDialog::Kind::Wireless, item);
        m_details->show();
    }

    // The switch shows the requested state until NetworkManager reports it,
    // so that a list refresh in between does not flip it back.
    void wirelessToggled(bool checked)
    {
        if (m_list.isMock() || checked == wirelessEnabled())
            return;
        m_requestedWireless = checked;
        ++m_wirelessRequests;
        m_handler.enableWireless(checked, [this](const Lingmo::base::dbus::Reply& reply) {
            --m_wirelessRequests;
            if (reply.isError())
                m_requestedWireless.reset();
            settleWireless();
            update();
        });
        if (checked)
            m_handler.requestScan();
        update();
    }

    bool wirelessEnabled() const { return m_requestedWireless.value_or(m_enabled.isWirelessEnabled()); }

    void settleWireless()
    {
        if (m_wirelessRequests == 0 && m_requestedWireless == m_enabled.isWirelessEnabled())
            m_requestedWireless.reset();
    }

    // The status bar sends enterprise networks here. The window and
    // NetworkManager's objects may not be there yet when the request arrives.
    void joinRequestedNetwork()
    {
        SettingsWindow& window = settingsWindow();
        if (!window.isRevealed() || !m_enabled.isReady())
            return;
        const std::string accessPoint = window.takePendingWifiJoin();
        if (accessPoint.empty())
            return;
        m_newNetwork.openNetwork(accessPoint, m_handler.wirelessNetwork(accessPoint));
    }

    void update()
    {
        const bool mock = m_list.isMock();
        content()->setVisible(m_enabled.isWirelessHwEnabled() || mock);
        m_switch->setChecked(mock || wirelessEnabled());

        const std::vector<nm::NetworkItem> items = m_list.rows(nm::ConnectionType::Wireless);
        // Switching networks leaves the old one Deactivating until the new one
        // is up; show it as already gone so only the network being joined spins.
        const bool activating = std::any_of(items.begin(), items.end(), [](const nm::NetworkItem& item) {
            return item.connectionState == nm::ConnectionStatus::Activating;
        });
        const auto displayed = [activating](const nm::NetworkItem& item) {
            if (item.connectionState == nm::ConnectionStatus::Deactivating && activating)
                return nm::ConnectionStatus::Deactivated;
            return item.connectionState;
        };

        std::vector<const nm::NetworkItem*> current;
        std::vector<const nm::NetworkItem*> known;
        std::vector<const nm::NetworkItem*> other;
        for (const nm::NetworkItem& item : items) {
            if (displayed(item) != nm::ConnectionStatus::Deactivated)
                current.push_back(&item);
            else if (!item.uuid.empty())
                known.push_back(&item);
            else
                other.push_back(&item);
        }

        const auto fill = [&displayed](KeyedRows<WifiRow>& rows, Card* card,
                                       const std::vector<const nm::NetworkItem*>& list) {
            std::vector<std::string> keys;
            for (const nm::NetworkItem* item : list)
                keys.push_back(item->key);
            const std::vector<WifiRow*> made = rows.update(card, keys);
            for (size_t i = 0; i < made.size(); ++i)
                made[i]->setItem(*list[i], displayed(*list[i]));
            return std::vector<View*>(made.begin(), made.end());
        };

        std::vector<View*> mainOrder{m_switch};
        for (View* row : fill(*m_currentRows, m_mainCard, current))
            mainOrder.push_back(row);
        mainOrder.push_back(m_hotspotRow);
        arrange(m_mainCard, mainOrder);

        const bool hotspot = m_handler.hotspotActive();
        m_hotspotRow->setVisible(hotspot);
        m_hotspotRow->setText(nm::HotspotConfiguration::instance()->name());

        std::vector<View*> knownOrder = fill(*m_known.rows, m_known.card, known);
        arrange(m_known.card, knownOrder);
        m_known.section->setVisible(!hotspot && !known.empty());

        std::vector<View*> otherOrder = fill(*m_other.rows, m_other.card, other);
        otherOrder.push_back(m_other.footer);
        arrange(m_other.card, otherOrder);
        m_other.footer->setVisible(m_switch->isChecked());
        m_other.section->setVisible(!hotspot && (!other.empty() || m_switch->isChecked()));
        requestLayout();
    }

    Lingmo::base::dbus::Proxy m_connectivity{Lingmo::base::dbus::Connection::session(),
            "org.lingmo.Services", "/org/lingmo/Services/Connectivity", "org.lingmo.Services.Connectivity"};
    Card* m_connectivityCard = nullptr;
    Label* m_connectivityLabel = nullptr;
    Button* m_login = nullptr;
    uint64_t m_connectivityGeneration = 0;
    Lingmo::base::dbus::Match m_connectivityProperties;
    Lingmo::base::dbus::NameWatch m_connectivityOwner;
    Lingmo::base::dbus::PendingCall m_connectivityRead;
    Lingmo::base::dbus::PendingCall m_loginCall;
    NetworkList m_list;
    nm::EnabledConnections m_enabled;
    nm::Handler m_handler;
    Lingmo::base::Timer m_scanTimer;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    NewNetworkDialog m_newNetwork;
    std::unique_ptr<DetailsDialog> m_details;
    std::optional<bool> m_requestedWireless;
    int m_wirelessRequests = 0;

    ColumnLayout* m_view;
    Card* m_mainCard;
    SwitchRow* m_switch;
    std::unique_ptr<KeyedRows<WifiRow>> m_currentRows;
    ListRow* m_hotspotRow;
    Label* m_hotspotLabel;
    TrailingIcon* m_hotspotChevron;
    WifiSection m_known;
    WifiSection m_other;
};

// Answered once, for the sample networks.
class FixedAvailability : public AvailabilityWatch
{
};

// Once NetworkManager has been read, Wi-Fi leaves the sidebar if no adapter is there.
class WirelessHardwareWatch : public AvailabilityWatch
{
public:
    explicit WirelessHardwareWatch(AvailabilityCallback callback)
        : m_callback(std::move(callback))
    {
        m_subscription = m_enabled.changed.connect([this] { report(); });
        report();
    }

private:
    void report()
    {
        if (m_enabled.isReady())
            m_callback(m_enabled.isWirelessHwEnabled());
    }

    nm::EnabledConnections m_enabled;
    Lingmo::base::Subscription m_subscription;
    AvailabilityCallback m_callback;
};

} // namespace

static const PageRegistration registration(
    "wlan", [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<WlanPage>(window); },
    [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
        // The sample networks show the page whatever the hardware.
        if (options().mockNetwork) {
            callback(true);
            return std::make_unique<FixedAvailability>();
        }
        return std::make_unique<WirelessHardwareWatch>(std::move(callback));
    });

} // namespace settings::network
