// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "lingmo/networkmanager/ip_utils.h"
#include "ui/cell_layout.h"

namespace Lingmo::Networkmanager {
class Handler;
}

namespace Lingmo::ui {
class ComboBox;
class DialogButton;
class Label;
class Switch;
class TextField;
}

namespace settings::network {

// How a saved network gets its addresses and name servers, shown in place of
// its details. Save writes the profile and applies it to the connection at
// once; onFinished follows a save or Cancel.
class IpSettingsView : public Lingmo::ui::ColumnLayout
{
public:
    explicit IpSettingsView(std::string connectionPath);
    ~IpSettingsView() override;

    std::function<void()> onFinished;

protected:
    void layout() override;

private:
    struct FormRow
    {
        Lingmo::ui::RowLayout* row;
        Lingmo::ui::Label* label;
    };

    FormRow addRow(Lingmo::ui::ColumnLayout* card, const std::string& label);
    Lingmo::ui::TextField* addField(Lingmo::ui::RowLayout* row, const std::string& placeholder);
    void load();
    Lingmo::Networkmanager::ip::Form collect() const;
    void fillFromCurrent(bool v4);
    void save();
    void update();

    std::string m_connectionPath;
    std::unique_ptr<Lingmo::Networkmanager::Handler> m_handler;
    std::vector<Lingmo::base::Subscription> m_subscriptions;
    Lingmo::Networkmanager::ip::Form m_form;
    bool m_saving = false;
    std::string m_error;
    std::vector<Lingmo::ui::Label*> m_fieldLabels;

    Lingmo::ui::ComboBox* m_ipv4Method;
    Lingmo::ui::RowLayout* m_ipv4AddressRow;
    Lingmo::ui::TextField* m_ipv4Address;
    Lingmo::ui::RowLayout* m_ipv4NetmaskRow;
    Lingmo::ui::TextField* m_ipv4Netmask;
    Lingmo::ui::RowLayout* m_ipv4GatewayRow;
    Lingmo::ui::TextField* m_ipv4Gateway;
    Lingmo::ui::ComboBox* m_ipv6Method;
    Lingmo::ui::RowLayout* m_ipv6AddressRow;
    Lingmo::ui::TextField* m_ipv6Address;
    Lingmo::ui::RowLayout* m_ipv6PrefixRow;
    Lingmo::ui::TextField* m_ipv6Prefix;
    Lingmo::ui::RowLayout* m_ipv6GatewayRow;
    Lingmo::ui::TextField* m_ipv6Gateway;
    Lingmo::ui::RowLayout* m_automaticDnsRow;
    Lingmo::ui::Switch* m_automaticDns;
    Lingmo::ui::TextField* m_dns;
    Lingmo::ui::Label* m_message;
    bool m_problem = false;
    Lingmo::ui::DialogButton* m_saveButton;
};

} // namespace settings::network
