// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "lingmo/networkmanager/connections.h"
#include "lingmo/networkmanager/handler.h"
#include "pages/network/common/detailsdialog.h"
#include "pages/network/common/networklist.h"
#include "pages/network/common/rows.h"
#include "ui/icon.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "window/options.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

namespace {

// One wired connection: a click activates or deactivates it; the info button
// opens its details.
class WiredRow : public ListRow
{
public:
    WiredRow()
    {
        m_icon = setLeading<Picture>();
        m_icon->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
        m_check = addContent<Icon>("check", 16);
        m_info = addContent<IconButton>("info", 20);
        m_info->onClicked = [this] {
            if (onInfo)
                onInfo(m_item);
        };
    }

    void setItem(const nm::NetworkItem& item)
    {
        m_item = item;
        setName("wired:" + item.name);
        setText(item.uniqueName);
        m_check->setVisible(item.connectionState == nm::ConnectionStatus::Activated);
    }

    const nm::NetworkItem& item() const { return m_item; }

    std::function<void(const nm::NetworkItem& item)> onInfo;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_icon->setPath(dataPath(std::string("images/") + (theme.darkMode ? "dark/" : "light/") + "network-wired.svg"));
        m_check->setColor(theme.highlight);
        m_info->setColor(theme.highlight);
        ListRow::layout();
    }

private:
    nm::NetworkItem m_item;
    Picture* m_icon;
    Icon* m_check;
    IconButton* m_info;
};

class EthernetPage : public Page
{
public:
    explicit EthernetPage(SettingsWindow& window)
        : Page(window, tr("Ethernet"))
    {
        // The switch and the wired connections share one card.
        m_card = content()->addChild<Card>(Card::Style::List);
        m_switch = m_card->addChild<SwitchRow>(tr("Ethernet"));
        m_switch->setName("ethernet:switch");
        m_switch->onToggled = [this](bool checked) {
            if (!m_list.isMock())
                m_handler.enableWired(checked);
        };
        m_rows = std::make_unique<KeyedRows<WiredRow>>([this](View* parent) {
            WiredRow* row = parent->addChild<WiredRow>();
            row->onClicked = [this, row] { toggle(row->item()); };
            row->onInfo = [this](const nm::NetworkItem& item) {
                m_details = std::make_unique<DetailsDialog>(settingsWindow(), m_list, DetailsDialog::Kind::Wired, item);
                m_details->show();
            };
            return row;
        });

        m_subscriptions.push_back(m_list.changed.connect([this] { update(); }));
        m_subscriptions.push_back(m_enabled.changed.connect([this] { update(); }));
        update();
    }

private:
    void toggle(const nm::NetworkItem& item)
    {
        if (m_list.isMock())
            return;
        if (item.connectionState == nm::ConnectionStatus::Deactivated)
            m_handler.activateConnection(item.connectionPath, item.devicePath, item.specificPath);
        else
            m_handler.deactivateConnection(item.connectionPath, item.devicePath);
    }

    void update()
    {
        const bool mock = m_list.isMock();
        m_card->setVisible(mock || m_enabled.isWiredAvailable());
        m_switch->setChecked(mock || m_enabled.isWiredEnabled());

        const std::vector<nm::NetworkItem> items = m_list.rows(nm::ConnectionType::Wired);
        std::vector<std::string> keys;
        for (const nm::NetworkItem& item : items)
            keys.push_back(item.key);
        const std::vector<WiredRow*> rows = m_rows->update(m_card, keys);
        std::vector<View*> order{m_switch};
        for (size_t i = 0; i < rows.size(); ++i) {
            rows[i]->setItem(items[i]);
            rows[i]->setVisible(mock || m_enabled.isWiredEnabled());
            order.push_back(rows[i]);
        }
        arrange(m_card, order);
        requestLayout();
    }

    NetworkList m_list;
    nm::EnabledConnections m_enabled;
    nm::Handler m_handler;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    std::unique_ptr<DetailsDialog> m_details;
    Card* m_card;
    SwitchRow* m_switch;
    std::unique_ptr<KeyedRows<WiredRow>> m_rows;
};

// The Ethernet page lists only connections on a present wired device;
// without one it would be blank.
class WiredConnectionsWatch : public AvailabilityWatch
{
public:
    explicit WiredConnectionsWatch(AvailabilityCallback callback)
        : m_callback(std::move(callback))
    {
        m_subscriptions.push_back(m_list.changed.connect([this] { report(); }));
        m_subscriptions.push_back(m_enabled.changed.connect([this] { report(); }));
        report();
    }

private:
    void report()
    {
        // Known only once NetworkManager has been read.
        if (!m_list.isMock() && !m_enabled.isReady())
            return;
        const bool available = !m_list.rows(nm::ConnectionType::Wired).empty();
        if (m_reported && available == m_available)
            return;
        m_reported = true;
        m_available = available;
        m_callback(available);
    }

    NetworkList m_list;
    nm::EnabledConnections m_enabled;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    AvailabilityCallback m_callback;
    bool m_reported = false;
    bool m_available = false;
};

} // namespace

static const PageRegistration registration(
    "ethernet", [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<EthernetPage>(window); },
    [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
        return std::make_unique<WiredConnectionsWatch>(std::move(callback));
    });

} // namespace settings::network
