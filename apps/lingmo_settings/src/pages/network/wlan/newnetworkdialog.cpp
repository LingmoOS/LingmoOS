// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/network/wlan/newnetworkdialog.h"

#include <algorithm>
#include <cctype>

#include "base/i18n.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "widgets/formdialog.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;
namespace nm = Lingmo::Networkmanager;

namespace settings::network {

static constexpr double kWidth = 400;

struct SecurityChoice
{
    const char* text;
    const char* value;
};

static const SecurityChoice kSecurities[] = {
    // TRANSLATORS: a Wi-Fi network's security: none, anyone can join.
    {trNoop("Open"), "none"},
    {"WEP", "wep"},
    {"WPA", "wpa"},
    {"WPA2/WPA3", "wpa2-wpa3"},
    {"WPA3", "wpa3"},
    {trNoop("WPA Enterprise"), "wpa-eap"},
    {trNoop("WPA2 Enterprise"), "wpa2-eap"},
    {trNoop("WPA3 Enterprise"), "wpa3-eap"},
};
static constexpr int kDefaultSecurity = 3;

static int indexOfSecurity(const std::string& value)
{
    for (size_t i = 0; i < std::size(kSecurities); ++i) {
        if (value == kSecurities[i].value)
            return int(i);
    }
    return kDefaultSecurity;
}

NewNetworkDialog::NewNetworkDialog(SettingsWindow& window)
    : m_window(window)
{
}

NewNetworkDialog::~NewNetworkDialog() = default;

void NewNetworkDialog::openNetwork(const std::string& accessPoint, const std::optional<nm::WirelessNetwork>& network)
{
    if (!network || network->device.empty())
        return;

    m_accessPoint = accessPoint;
    m_device = network->device;
    m_networkName = network->name;
    m_needsCertificate = network->security == nm::SecurityType::Wpa3SuiteB192;
    build();
    m_security->setCurrentIndex(indexOfSecurity(network->security == nm::SecurityType::WpaEap    ? "wpa-eap"
                                                : network->security == nm::SecurityType::Wpa2Eap ? "wpa2-eap"
                                                                                                 : "wpa3-eap"));
    update();
    m_dialog->show();
    m_userName->setFocus();
}

void NewNetworkDialog::show()
{
    if (isOpen())
        return;
    m_accessPoint.clear();
    m_device.clear();
    m_networkName.clear();
    m_needsCertificate = false;
    build();
    update();
    m_dialog->show();
    m_name->setFocus();
}

void NewNetworkDialog::close()
{
    if (m_dialog)
        m_dialog->close();
}

bool NewNetworkDialog::isOpen() const
{
    return m_dialog && m_dialog->isOpen();
}

std::string NewNetworkDialog::title() const
{
    return m_accessPoint.empty() ? std::string(tr("Add Other Network"))
                                 : format(tr("Join “{network}”"), {{"network", m_networkName}});
}

std::string NewNetworkDialog::security() const
{
    const int index = m_security ? m_security->currentIndex() : -1;
    return index >= 0 && size_t(index) < std::size(kSecurities) ? kSecurities[index].value : "";
}

static bool isHex(const std::string& text)
{
    return std::all_of(text.begin(), text.end(), [](char c) { return std::isxdigit(static_cast<unsigned char>(c)); });
}

// What NetworkManager accepts for each kind of key: a passphrase, or the key
// itself in hexadecimal.
bool NewNetworkDialog::passwordValid(const std::string& password) const
{
    const std::string type = security();
    if (type == "none")
        return true;
    const size_t length = password.size();
    if (type == "wep")
        return length == 5 || length == 13 || ((length == 10 || length == 26) && isHex(password));
    if (type == "wpa" || type == "wpa2-wpa3" || type == "wpa3")
        return (length >= 8 && length <= 63) || (length == 64 && isHex(password));
    return !password.empty();
}

bool NewNetworkDialog::canJoin() const
{
    if (!m_dialog)
        return false;
    const bool enterprise = security().ends_with("-eap");
    return !m_needsCertificate && (!m_accessPoint.empty() || !m_name->text().empty())
           && (!enterprise || !m_userName->text().empty()) && passwordValid(m_password->text());
}

void NewNetworkDialog::join()
{
    if (!canJoin())
        return;

    if (!m_accessPoint.empty()) {
        if (onJoinNetwork)
            onJoinNetwork(m_device, m_accessPoint, m_userName->text(), m_password->text());
    } else if (onConnect) {
        const std::string type = security();
        onConnect(m_name->text(), type.ends_with("-eap") ? m_userName->text() : std::string(),
                  type == "none" ? std::string() : m_password->text(), type);
    }
    close();
}

void NewNetworkDialog::build()
{
    m_dialog = std::make_unique<FormDialog>(m_window, title(), kWidth);
    m_dialog->onClosed = [this] {
        // Closing returns the dialog to a hidden network.
        m_accessPoint.clear();
        m_device.clear();
        m_networkName.clear();
        m_needsCertificate = false;
    };
    ColumnLayout* content = m_dialog->content();

    m_form = content->addChild<GridLayout>(2);
    m_form->setColumnSpacing(metrics::dialogSpacing);
    m_form->setRowSpacing(metrics::formRowSpacing);
    m_form->setLayoutHints({.fillWidth = true});
    m_labels.clear();

    const auto addLabel = [this](const char* text) {
        Label* label = m_form->addChild<Label>(text);
        m_labels.push_back(label);
        return label;
    };
    const auto addField = [this](const char* placeholder) {
        TextField* field = m_form->addChild<TextField>();
        field->setPlaceholder(placeholder);
        field->setLayoutHints({.fillWidth = true});
        field->onTextChanged = [this] { update(); };
        field->onAccepted = [this] { join(); };
        return field;
    };

    m_nameLabel = addLabel(tr("Name"));
    m_name = addField(tr("Network name"));
    m_securityLabel = addLabel(tr("Security"));
    std::vector<std::string> securities;
    for (const SecurityChoice& choice : kSecurities)
        securities.push_back(tr(choice.text));
    m_security = m_form->addChild<ComboBox>(securities);
    m_security->setLayoutHints({.fillWidth = true});
    m_security->setCurrentIndex(kDefaultSecurity);
    m_security->onActivated = [this](int) { update(); };
    m_userNameLabel = addLabel(tr("Username"));
    m_userName = addField(tr("Username"));
    m_passwordLabel = addLabel(tr("Password"));
    m_password = addField(tr("Password"));
    m_password->setEchoMode(TextField::EchoMode::Password);

    // Hidden rows leave the grid, so the fields would move as they appear.
    double labelWidth = 0;
    for (const Label* label : m_labels)
        labelWidth = std::max(labelWidth, label->sizeHint().width);
    for (Label* label : m_labels)
        label->setLayoutHints({.preferredWidth = labelWidth});

    m_certificateNote =
            content->addChild<Label>(tr("This network signs in with a certificate, which can't be set up here yet."));
    m_certificateNote->setWrap(true);
    m_certificateNote->setLayoutHints({.fillWidth = true});

    RowLayout* buttons = m_dialog->addButtonRow();
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { close(); };
    m_joinButton = buttons->addChild<DialogButton>(tr("Join"), true);
    m_joinButton->onClicked = [this] { join(); };
}

void NewNetworkDialog::update()
{
    if (!m_dialog)
        return;
    const std::string type = security();
    const bool listed = !m_accessPoint.empty();
    const bool enterprise = type.ends_with("-eap");
    m_dialog->setTitle(title());
    m_nameLabel->setVisible(!listed);
    m_name->setVisible(!listed);
    m_securityLabel->setVisible(!listed);
    m_security->setVisible(!listed);
    m_userNameLabel->setVisible(enterprise && !m_needsCertificate);
    m_userName->setVisible(enterprise && !m_needsCertificate);
    m_passwordLabel->setVisible(type != "none" && !m_needsCertificate);
    m_password->setVisible(type != "none" && !m_needsCertificate);
    m_certificateNote->setVisible(m_needsCertificate);
    m_joinButton->setEnabled(canJoin());
}

} // namespace settings::network
