// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/common/ipsettingsview.h"

#include <algorithm>
#include <cctype>

#include "base/i18n.h"
#include "lingmo/networkmanager/handler.h"
#include "pages/network/common/rows.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

// A label column for every card, as wide as the longest label up to this.
static constexpr double kMaximumLabelWidth = 140;

static int indexOfMethod(const std::vector<nm::ip::Method>& methods, const std::string& value)
{
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i].value == value)
            return int(i);
    }
    return 0;
}

static std::vector<std::string> methodLabels(const std::vector<nm::ip::Method>& methods)
{
    std::vector<std::string> labels;
    for (const nm::ip::Method& method : methods)
        labels.push_back(method.label);
    return labels;
}

static std::string currentValue(const ComboBox* box, const std::vector<nm::ip::Method>& methods)
{
    const int index = box->currentIndex();
    return index >= 0 && size_t(index) < methods.size() ? methods[size_t(index)].value : std::string();
}

static Card* addCard(ColumnLayout* parent)
{
    Card* card = parent->addChild<Card>();
    card->setPadding(Margins::all(metrics::cardHorizontalPadding));
    card->setSpacing(metrics::formRowSpacing);
    return card;
}

IpSettingsView::IpSettingsView(std::string connectionPath)
    : m_connectionPath(std::move(connectionPath))
    , m_handler(std::make_unique<nm::Handler>())
{
    setSpacing(metrics::dialogSpacing);
    setLayoutHints({.fillWidth = true});

    m_subscriptions.push_back(m_handler->ipSaved.connect([this](const std::string& connection) {
        if (connection != m_connectionPath || !m_saving)
            return;
        m_saving = false;
        if (onFinished)
            onFinished();
    }));
    m_subscriptions.push_back(
        m_handler->ipSaveFailed.connect([this](const std::string& connection, const std::string& message) {
            if (connection != m_connectionPath || !m_saving)
                return;
            m_saving = false;
            m_error = message;
            update();
        }));

    Card* ipv4 = addCard(this);
    m_ipv4Method = addRow(ipv4, tr("Configure IPv4")).row->addChild<ComboBox>();
    m_ipv4Method->setLayoutHints({.fillWidth = true});
    m_ipv4Method->onActivated = [this](int) {
        if (currentValue(m_ipv4Method, m_form.ipv4Methods) == "manual")
            fillFromCurrent(true);
        update();
    };
    FormRow row = addRow(ipv4, tr("IP Address"));
    m_ipv4AddressRow = row.row;
    m_ipv4Address = addField(row.row, tr("Required"));
    row = addRow(ipv4, tr("Subnet Mask"));
    m_ipv4NetmaskRow = row.row;
    m_ipv4Netmask = addField(row.row, "255.255.255.0");
    row = addRow(ipv4, tr("Router"));
    m_ipv4GatewayRow = row.row;
    m_ipv4Gateway = addField(row.row, tr("Optional"));

    Card* ipv6 = addCard(this);
    m_ipv6Method = addRow(ipv6, tr("Configure IPv6")).row->addChild<ComboBox>();
    m_ipv6Method->setLayoutHints({.fillWidth = true});
    m_ipv6Method->onActivated = [this](int) {
        if (currentValue(m_ipv6Method, m_form.ipv6Methods) == "manual")
            fillFromCurrent(false);
        update();
    };
    row = addRow(ipv6, tr("IP Address"));
    m_ipv6AddressRow = row.row;
    m_ipv6Address = addField(row.row, tr("Required"));
    row = addRow(ipv6, tr("Prefix Length"));
    m_ipv6PrefixRow = row.row;
    m_ipv6Prefix = addField(row.row, "64");
    row = addRow(ipv6, tr("Router"));
    m_ipv6GatewayRow = row.row;
    m_ipv6Gateway = addField(row.row, tr("Optional"));

    Card* dns = addCard(this);
    m_automaticDnsRow = dns->addChild<RowLayout>();
    m_automaticDnsRow->setSpacing(metrics::dialogSpacing);
    m_automaticDnsRow->setLayoutHints({.fillWidth = true});
    Label* automaticLabel = m_automaticDnsRow->addChild<Label>(tr("Automatic DNS"));
    automaticLabel->setWrap(true);
    automaticLabel->setLayoutHints({.fillWidth = true});
    m_automaticDns = m_automaticDnsRow->addChild<Switch>();
    setTrailingSwitchPadding(m_automaticDns);
    m_automaticDns->onToggled = [this](bool) { update(); };
    row = addRow(dns, tr("DNS Servers"));
    m_dns = addField(row.row, {});

    m_message = addChild<Label>();
    m_message->setWrap(true);
    m_message->setLayoutHints({.fillWidth = true});

    RowLayout* buttons = addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] {
        if (onFinished)
            onFinished();
    };
    m_saveButton = buttons->addChild<DialogButton>(tr("Save"), true);
    m_saveButton->onClicked = [this] { save(); };

    load();
}

IpSettingsView::~IpSettingsView() = default;

IpSettingsView::FormRow IpSettingsView::addRow(ColumnLayout* card, const std::string& text)
{
    RowLayout* row = card->addChild<RowLayout>();
    row->setSpacing(metrics::dialogSpacing);
    row->setLayoutHints({.fillWidth = true});
    Label* label = row->addChild<Label>(text);
    label->setWrap(true);
    m_fieldLabels.push_back(label);
    return {row, label};
}

TextField* IpSettingsView::addField(RowLayout* row, const std::string& placeholder)
{
    TextField* field = row->addChild<TextField>();
    field->setPlaceholder(placeholder);
    field->setLayoutHints({.fillWidth = true});
    field->onTextChanged = [this] { update(); };
    field->onAccepted = [this] { save(); };
    return field;
}

void IpSettingsView::load()
{
    m_form = m_handler->ipForm(m_connectionPath);
    m_ipv4Method->setItems(methodLabels(m_form.ipv4Methods));
    m_ipv4Method->setCurrentIndex(indexOfMethod(m_form.ipv4Methods, m_form.ipv4Method));
    m_ipv4Address->setText(m_form.ipv4Address);
    m_ipv4Netmask->setText(m_form.ipv4Netmask);
    m_ipv4Gateway->setText(m_form.ipv4Gateway);
    m_ipv6Method->setItems(methodLabels(m_form.ipv6Methods));
    m_ipv6Method->setCurrentIndex(indexOfMethod(m_form.ipv6Methods, m_form.ipv6Method));
    m_ipv6Address->setText(m_form.ipv6Address);
    m_ipv6Prefix->setText(m_form.ipv6Prefix);
    m_ipv6Gateway->setText(m_form.ipv6Gateway);
    m_automaticDns->setChecked(m_form.automaticDns);
    m_dns->setText(m_form.dns);
    update();
}

nm::ip::Form IpSettingsView::collect() const
{
    nm::ip::Form form;
    form.ipv4Method = currentValue(m_ipv4Method, m_form.ipv4Methods);
    form.ipv4Address = m_ipv4Address->text();
    form.ipv4Netmask = m_ipv4Netmask->text();
    form.ipv4Gateway = m_ipv4Gateway->text();
    form.ipv6Method = currentValue(m_ipv6Method, m_form.ipv6Methods);
    form.ipv6Address = m_ipv6Address->text();
    form.ipv6Prefix = m_ipv6Prefix->text();
    form.ipv6Gateway = m_ipv6Gateway->text();
    form.automaticDns = m_automaticDns->isChecked();
    form.dns = m_dns->text();
    return form;
}

// A manual setup starts from the addresses the connection has now, as they
// usually are what is to be kept.
void IpSettingsView::fillFromCurrent(bool v4)
{
    const nm::ip::Form current = m_handler->currentAddresses(m_connectionPath);
    TextField* address = v4 ? m_ipv4Address : m_ipv6Address;
    const std::string& currentAddress = v4 ? current.ipv4Address : current.ipv6Address;
    if (!address->text().empty() || currentAddress.empty())
        return;
    address->setText(currentAddress);
    (v4 ? m_ipv4Netmask : m_ipv6Prefix)->setText(v4 ? current.ipv4Netmask : current.ipv6Prefix);
    (v4 ? m_ipv4Gateway : m_ipv6Gateway)->setText(v4 ? current.ipv4Gateway : current.ipv6Gateway);
}

void IpSettingsView::save()
{
    if (m_saving || !nm::ip::check(collect()).complete)
        return;
    m_saving = true;
    m_error.clear();
    update();
    m_handler->saveIp(m_connectionPath, collect());
}

void IpSettingsView::update()
{
    const nm::ip::Form form = collect();
    const bool ipv4Manual = form.ipv4Method == "manual";
    const bool ipv6Manual = form.ipv6Method == "manual";
    // Only a family that asks the network for its addresses can be told its name servers too.
    const bool automaticFamily = form.ipv4Method == "auto" || form.ipv4Method == "dhcp" || form.ipv6Method == "auto"
                                 || form.ipv6Method == "dhcp";
    for (RowLayout* row : {m_ipv4AddressRow, m_ipv4NetmaskRow, m_ipv4GatewayRow})
        row->setVisible(ipv4Manual);
    for (RowLayout* row : {m_ipv6AddressRow, m_ipv6PrefixRow, m_ipv6GatewayRow})
        row->setVisible(ipv6Manual);
    m_automaticDnsRow->setVisible(automaticFamily);
    m_dns->setPlaceholder(automaticFamily && m_automaticDns->isChecked() ? tr("Optional") : "1.1.1.1, 8.8.8.8");

    const nm::ip::Check result = nm::ip::check(form);
    const std::string problem = !m_error.empty() ? m_error : result.message;
    m_problem = !problem.empty();
    std::string message = problem;
    bool hasDns = false;
    for (const char c : m_dns->text())
        hasDns |= !std::isspace(static_cast<unsigned char>(c));
    if (message.empty() && automaticFamily && m_automaticDns->isChecked() && hasDns)
        message = tr("These servers are asked before the network's own.");
    m_message->setText(message);
    m_message->setVisible(!message.empty());

    m_saveButton->setText(m_saving ? tr("Saving…") : tr("Save"));
    m_saveButton->setEnabled(!m_saving && result.complete);
    requestLayout();
}

void IpSettingsView::layout()
{
    const Theme& theme = Theme::current();
    m_message->setColor(m_problem ? Lingmo::ui::accent::red : theme.disabledText);

    double labelWidth = 0;
    for (const Label* label : m_fieldLabels)
        labelWidth = std::max(labelWidth, std::min(label->sizeHint().width, kMaximumLabelWidth));
    for (Label* label : m_fieldLabels) {
        LayoutHints hints = label->layoutHints();
        if (hints.preferredWidth != labelWidth) {
            hints.preferredWidth = labelWidth;
            label->setLayoutHints(hints);
        }
    }
    ColumnLayout::layout();
}

} // namespace settings::network
