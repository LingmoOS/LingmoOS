// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/i18n.h"
#include "lingmo/networkmanager/handler.h"
#include "pages/network/common/networklist.h"
#include "pages/network/common/rows.h"
#include "pages/network/vpn/vpndialogs.h"
#include "ui/file_chooser.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/progress_bar.h"
#include "ui/cell_layout.h"
#include "ui/switch.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

namespace {

// One VPN: its switch connects and disconnects it; below its name, its type,
// or why it can't connect.
class VpnRow : public ListRow
{
public:
    VpnRow()
    {
        setLeading<Icon>("network-vpn", 18);
        ColumnLayout* texts = addContent<ColumnLayout>();
        texts->setSpacing(metrics::textSpacing);
        texts->setLayoutHints({.fillWidth = true});
        m_name = texts->addChild<Label>();
        m_name->setLayoutHints({.fillWidth = true});
        m_detail = texts->addChild<Label>();
        // A point smaller than the name, as the row is measured before it is laid out.
        m_detail->setFontSize(std::max(1.0, Theme::current().fontSize() - 1));
        m_detail->setWrap(true);
        m_detail->setLayoutHints({.fillWidth = true});
        m_busy = addContent<BusyIndicator>();
        m_busy->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
        m_switch = addContent<Switch>();
        setTrailingSwitchPadding(m_switch);
        m_switch->onToggled = [this](bool checked) {
            if (onSwitched)
                onSwitched(checked);
            // The switch shows what NetworkManager reports, not what was asked for.
            m_switch->setChecked(isUp());
        };
        m_info = addContent<IconButton>("info", 20);
        m_info->onClicked = [this] {
            if (onClicked)
                onClicked();
        };
    }

    void setItem(const nm::NetworkItem& item)
    {
        m_item = item;
        setName("vpn:" + item.name);
        const bool busy = item.connectionState == nm::ConnectionStatus::Activating
                          || item.connectionState == nm::ConnectionStatus::Deactivating;
        m_name->setText(item.name);
        m_detail->setText(!item.statusMessage.empty()                               ? item.statusMessage
                          : item.connectionState == nm::ConnectionStatus::Activating ? std::string(tr("Connecting…"))
                          : needsUserName()                                          ? std::string(tr("Add your user name to connect."))
                                                                                     : item.vpnType);
        m_problem = !item.statusMessage.empty() && item.canActivate;
        m_busy->setVisible(busy);
        m_busy->setRunning(busy);
        m_switch->setEnabled(item.canActivate || isUp());
        m_switch->setChecked(isUp());
        requestLayout();
    }

    const nm::NetworkItem& item() const { return m_item; }

    bool isUp() const
    {
        return m_item.connectionState == nm::ConnectionStatus::Activated
               || m_item.connectionState == nm::ConnectionStatus::Activating;
    }

    // Signing in takes a user name the imported file didn't have.
    bool needsUserName() const { return m_item.vpnUsesUserName && m_item.vpnUserName.empty(); }

    std::function<void(bool checked)> onSwitched;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_name->setColor(isEnabled() ? theme.text : theme.disabledText);
        m_detail->setColor(m_problem ? accent::red : theme.disabledText);
        m_detail->setFontSize(std::max(1.0, theme.fontSize() - 1));
        m_info->setColor(theme.highlight);
        ListRow::layout();
    }

private:
    nm::NetworkItem m_item;
    Label* m_name;
    Label* m_detail;
    bool m_problem = false;
    BusyIndicator* m_busy;
    Switch* m_switch;
    IconButton* m_info;
};

// The VPNs set up on this machine, each with its own switch, and ways to add
// one: by hand, or from the configuration file a VPN provider hands out.
class VpnPage : public Page
{
public:
    explicit VpnPage(SettingsWindow& window)
        : Page(window, tr("VPN"))
        , m_addDialog(window, m_handler)
        , m_importChooser(&window.window())
    {
        content()->setSpacing(metrics::sectionLabelSpacing);
        m_card = content()->addChild<Card>(Card::Style::List);
        m_rows = std::make_unique<KeyedRows<VpnRow>>([this](View* parent) {
            VpnRow* row = parent->addChild<VpnRow>();
            row->onClicked = [this, row] { openDetails(row->item()); };
            row->onSwitched = [this, row](bool checked) {
                const nm::NetworkItem item = row->item();
                if (checked && row->needsUserName())
                    openDetails(item);
                else if (checked)
                    m_handler.activateConnection(item.connectionPath, {}, {});
                else
                    m_handler.deactivateConnection(item.connectionPath, {});
            };
            return row;
        });
        m_add = m_card->addChild<ListRow>(tr("Add VPN…"));
        m_add->setName("vpn:add");
        m_add->onClicked = [this] { m_addDialog.show(); };
        m_import = m_card->addChild<ListRow>(tr("Import from file…"));
        m_import->setName("vpn:import");
        m_import->onClicked = [this] {
            m_importChooser.open();
        };
        m_importChooser.setTitle(tr("Import VPN Configuration"));
        m_importChooser.setFilters({FileChooser::Filter::fromNameFilter(format(tr("VPN configurations ({patterns})"),
                                                                               {{"patterns", "*.ovpn *.conf *.pcf"}})),
                                    FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
        m_importChooser.onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
            if (paths.empty())
                return;
            m_importing = true;
            m_importError.clear();
            update();
            m_handler.importVpn(paths.front());
        };

        m_note = content()->addChild<Label>();
        m_note->setName("vpn:note");
        m_note->setWrap(true);
        m_note->setLayoutHints(
            {.fillWidth = true, .margins = {metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0}});

        m_subscriptions.push_back(m_handler.vpnAdded.connect([this](const std::string&) {
            m_importing = false;
            m_importError.clear();
            update();
        }));
        m_subscriptions.push_back(m_handler.vpnAddFailed.connect([this](const std::string& message) {
            // The Add VPN dialog shows its own failures.
            if (!m_importing)
                return;
            m_importing = false;
            m_importError = message;
            update();
        }));
        m_subscriptions.push_back(m_list.changed.connect([this] { update(); }));
        update();
    }

protected:
    void layout() override
    {
        m_note->setColor(m_importError.empty() ? Theme::current().disabledText : accent::red);
        Page::layout();
    }

private:
    void openDetails(const nm::NetworkItem& item)
    {
        m_details = std::make_unique<VpnDetailsDialog>(settingsWindow(), m_list, m_handler, item.connectionPath);
        m_details->show();
    }

    void update()
    {
        const std::vector<nm::NetworkItem> items = m_list.rows(nm::ConnectionType::Vpn);
        std::vector<std::string> keys;
        for (const nm::NetworkItem& item : items)
            keys.push_back(item.key);
        const std::vector<VpnRow*> rows = m_rows->update(m_card, keys);
        std::vector<View*> order;
        for (size_t i = 0; i < rows.size(); ++i) {
            rows[i]->setItem(items[i]);
            order.push_back(rows[i]);
        }
        order.push_back(m_add);
        order.push_back(m_import);
        arrange(m_card, order);

        m_add->setEnabled(!m_importing);
        m_import->setEnabled(!m_importing);
        m_import->setText(m_importing ? tr("Importing…") : tr("Import from file…"));
        m_note->setText(!m_importError.empty() ? m_importError
                                               : tr("Import the configuration file your VPN provider gives you, such "
                                                    "as an OpenVPN .ovpn or a WireGuard .conf file."));
        requestLayout();
    }

    NetworkList m_list;
    nm::Handler m_handler;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    AddVpnDialog m_addDialog;
    std::unique_ptr<VpnDetailsDialog> m_details;
    FileChooser m_importChooser;
    bool m_importing = false;
    std::string m_importError;
    Card* m_card;
    std::unique_ptr<KeyedRows<VpnRow>> m_rows;
    ListRow* m_add;
    ListRow* m_import;
    Label* m_note;
};

} // namespace

static const PageRegistration registration("vpn", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<VpnPage>(window);
});

} // namespace settings::network
