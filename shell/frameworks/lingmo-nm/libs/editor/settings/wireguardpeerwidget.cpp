/*
    SPDX-FileCopyrightText: 2019 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "wireguardpeerwidget.h"
#include "simpleiplistvalidator.h"
#include "simpleipv4addressvalidator.h"
#include "ui_wireguardpeerwidget.h"
#include "wireguardkeyvalidator.h"

#include <QStandardItemModel>
#include <QUrl>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/Utils>

// Keys for the NetworkManager configuration
#define PNM_SETTING_WIREGUARD_SETTING_NAME "wireguard"

#define PNM_WG_KEY_PEERS "peers"
#define PNM_WG_KEY_MTU "mtu"
#define PNM_WG_KEY_PEER_ROUTES "peer-routes"
#define PNM_WG_PEER_KEY_ALLOWED_IPS "allowed-ips"
#define PNM_WG_PEER_KEY_ENDPOINT "endpoint"
#define PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE "persistent-keepalive"
#define PNM_WG_PEER_KEY_PRESHARED_KEY "preshared-key"
#define PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS "preshared-key-flags"
#define PNM_WG_PEER_KEY_PUBLIC_KEY "public-key"

static WireGuardKeyValidator keyValidator;
static SimpleIpListValidator allowedIPsValidator(SimpleIpListValidator::WithCidr, SimpleIpListValidator::Both);

class WireGuardPeerWidget::Private
{
public:
    ~Private();

    Ui_WireGuardPeersProp ui;
    NetworkManager::WireGuardSetting::Ptr setting;
    KSharedConfigPtr config;
    QPalette warningPalette;
    QPalette normalPalette;
    QVariantMap peerData;
    bool publicKeyValid = false;
    bool allowedIPsValid = false;
    bool endpointValid = true;
    bool presharedKeyValid = true;
};

WireGuardPeerWidget::Private::~Private() = default;

WireGuardPeerWidget::WireGuardPeerWidget(const QVariantMap &peerData, QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
    , d(new Private)
{
    d->ui.setupUi(this);
    d->peerData = peerData;

    d->config = KSharedConfig::openConfig();
    d->warningPalette = KColorScheme::createApplicationPalette(d->config);
    d->normalPalette = KColorScheme::createApplicationPalette(d->config);
    KColorScheme::adjustBackground(d->warningPalette, KColorScheme::NegativeBackground, QPalette::Base, KColorScheme::ColorSet::View, d->config);

    KColorScheme::adjustBackground(d->normalPalette, KColorScheme::NormalBackground, QPalette::Base, KColorScheme::ColorSet::View, d->config);

    setWindowTitle(i18nc("@title: window wireguard peers properties", "WireGuard peers properties"));
    connect(d->ui.publicKeyLineEdit, &QLineEdit::textChanged, this, &WireGuardPeerWidget::checkPublicKeyValid);
    connect(d->ui.allowedIPsLineEdit, &QLineEdit::textChanged, this, &WireGuardPeerWidget::checkAllowedIpsValid);
    connect(d->ui.endpointAddressLineEdit, &QLineEdit::textChanged, this, &WireGuardPeerWidget::checkEndpointValid);
    connect(d->ui.endpointPortLineEdit, &QLineEdit::textChanged, this, &WireGuardPeerWidget::checkEndpointValid);
    connect(d->ui.presharedKeyLineEdit, &PasswordField::textChanged, this, &WireGuardPeerWidget::checkPresharedKeyValid);
    connect(d->ui.presharedKeyLineEdit, &PasswordField::passwordOptionChanged, this, &WireGuardPeerWidget::saveKeyFlags);
    connect(d->ui.keepaliveLineEdit, &QLineEdit::textChanged, this, &WireGuardPeerWidget::saveKeepAlive);

    d->ui.presharedKeyLineEdit->setPasswordModeEnabled(true);
    d->ui.presharedKeyLineEdit->setPasswordOptionsEnabled(true);
    d->ui.presharedKeyLineEdit->setPasswordNotRequiredEnabled(true);
    d->ui.presharedKeyLineEdit->setPasswordNotSavedEnabled(false);

    // Create validator for endpoint port
    auto portValidator = new QIntValidator(this);
    portValidator->setBottom(0);
    portValidator->setTop(65535);
    d->ui.endpointPortLineEdit->setValidator(portValidator);

    // Reuse the port validator for the persistent keepalive.
    // It is not a "port" but has the same limits
    d->ui.keepaliveLineEdit->setValidator(portValidator);

    KAcceleratorManager::manage(this);
    updatePeerWidgets();

    // Set the initial backgrounds on all the widgets
    checkPublicKeyValid();
    checkAllowedIpsValid();
    checkEndpointValid();
}

WireGuardPeerWidget::~WireGuardPeerWidget()
{
    delete d;
}

QVariantMap WireGuardPeerWidget::setting() const
{
    return d->peerData;
}

void WireGuardPeerWidget::checkPublicKeyValid()
{
    int pos = 0;
    QLineEdit *widget = d->ui.publicKeyLineEdit;
    QString value = widget->displayText();
    bool valid = QValidator::Acceptable == keyValidator.validate(value, pos);
    setBackground(widget, valid);
    d->peerData[PNM_WG_PEER_KEY_PUBLIC_KEY] = value;
    if (valid != d->publicKeyValid) {
        d->publicKeyValid = valid;
        slotWidgetChanged();
    }
}

void WireGuardPeerWidget::checkPresharedKeyValid()
{
    int pos = 0;
    PasswordField *widget = d->ui.presharedKeyLineEdit;
    QString value = widget->text();

    // The value in the preshared key field is ignored if the type
    // of password is set to "Ask every time" or "Not Required" so
    // it is valid if it is set to "Store for user only"
    // or "Store for all users" even if the password is bad
    PasswordField::PasswordOption option = d->ui.presharedKeyLineEdit->passwordOption();
    bool valid = (QValidator::Acceptable == keyValidator.validate(value, pos) || option == PasswordField::NotRequired);
    setBackground(widget, valid);
    if (value.isEmpty())
        d->peerData.remove(PNM_WG_PEER_KEY_PRESHARED_KEY);
    else
        d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY] = value;
    if (valid != d->presharedKeyValid) {
        d->presharedKeyValid = valid;
        slotWidgetChanged();
    }
}

void WireGuardPeerWidget::checkAllowedIpsValid()
{
    int pos = 0;
    QLineEdit *widget = d->ui.allowedIPsLineEdit;
    QString ipString = widget->displayText();
    QStringList rawIPList = ipString.split(QLatin1Char(','));
    QStringList ipList;

    bool valid = QValidator::Acceptable == allowedIPsValidator.validate(ipString, pos);
    setBackground(widget, valid);

    ipList.reserve(rawIPList.size());
    for (const QString &ip : rawIPList) {
        ipList.append(ip.trimmed());
    }

    d->peerData[PNM_WG_PEER_KEY_ALLOWED_IPS] = ipList;
    if (valid != d->allowedIPsValid) {
        d->allowedIPsValid = valid;
        slotWidgetChanged();
    }
}

WireGuardPeerWidget::EndPointValid WireGuardPeerWidget::isEndpointValid(QString &address, QString &port)
{
    // Create a Reg Expression validator to do a simple check for a valid qualified domain name
    // which checks to see that there are between 2 and 63 strings of at least 1 character each
    // separated by '.'. The full string must also be less than 255 characters long.
    static QRegularExpressionValidator fqdnValidator(QRegularExpression(QLatin1String("(?=.{3,254}$)([a-zA-Z0-9][a-zA-Z0-9-]{0,62}\\.){1,63}[a-zA-Z]{1,63}")),
                                                     nullptr);
    static SimpleIpV4AddressValidator ipv4Validator;
    static SimpleIpV6AddressValidator ipv6Validator;
    int pos = 0;

    bool addressValid = QValidator::Acceptable == fqdnValidator.validate(address, pos) || QValidator::Acceptable == ipv4Validator.validate(address, pos)
        || QValidator::Acceptable == ipv6Validator.validate(address, pos);
    bool bothEmpty = address.isEmpty() && port.isEmpty();
    // Because of the validator, if the port is non-empty, it is valid
    bool portValid = !port.isEmpty();

    if ((portValid && addressValid) || bothEmpty)
        return WireGuardPeerWidget::BothValid;
    else if (portValid)
        return WireGuardPeerWidget::PortValid;
    else if (addressValid)
        return WireGuardPeerWidget::AddressValid;
    else
        return WireGuardPeerWidget::BothInvalid;
}

void WireGuardPeerWidget::checkEndpointValid()
{
    QLineEdit *addressWidget = d->ui.endpointAddressLineEdit;
    QLineEdit *portWidget = d->ui.endpointPortLineEdit;
    QString addressString = addressWidget->displayText();
    QString portString = portWidget->displayText();

    WireGuardPeerWidget::EndPointValid valid = isEndpointValid(addressString, portString);

    setBackground(addressWidget, WireGuardPeerWidget::BothValid == valid || WireGuardPeerWidget::AddressValid == valid);
    setBackground(portWidget, WireGuardPeerWidget::BothValid == valid || WireGuardPeerWidget::PortValid == valid);

    // If there is a ':' in the address string then it is an IPv6 address and
    // the output needs to be formatted as '[1:2:3:4:5:6:7:8]:123' otherwise
    // it is formatted as '1.2.3.4:123' or 'ab.com:123'
    QString stringToStore;
    if (addressString.contains(":"))
        stringToStore = "[" + addressString.trimmed() + "]:" + portString.trimmed();
    else
        stringToStore = addressString.trimmed() + ":" + portString.trimmed();

    if (addressString.isEmpty() && portString.isEmpty())
        d->peerData.remove(PNM_WG_PEER_KEY_ENDPOINT);
    else
        d->peerData[PNM_WG_PEER_KEY_ENDPOINT] = stringToStore;

    if ((valid == WireGuardPeerWidget::BothValid) != d->endpointValid) {
        d->endpointValid = (valid == WireGuardPeerWidget::BothValid);
        slotWidgetChanged();
    }
}

bool WireGuardPeerWidget::isValid()
{
    return d->publicKeyValid && d->allowedIPsValid && d->endpointValid && d->presharedKeyValid;
}

void WireGuardPeerWidget::saveKeepAlive()
{
    QLineEdit *widget = d->ui.keepaliveLineEdit;
    QString value = widget->displayText();

    if (value.isEmpty())
        d->peerData.remove(PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE);
    else
        d->peerData[PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE] = value;
}

void WireGuardPeerWidget::saveKeyFlags()
{
    PasswordField::PasswordOption option = d->ui.presharedKeyLineEdit->passwordOption();
    switch (option) {
    case PasswordField::StoreForUser:
        d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS] = NetworkManager::Setting::AgentOwned;
        break;
    case PasswordField::StoreForAllUsers:
        d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS] = NetworkManager::Setting::None;
        break;
    // Always Ask is not a valid option for the preshared key so set it to AgentOwned instead
    case PasswordField::AlwaysAsk:
        d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS] = NetworkManager::Setting::AgentOwned;
        break;
    case PasswordField::NotRequired:
        d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS] = NetworkManager::Setting::NotRequired;
        break;
    }
    checkPresharedKeyValid();
}

void WireGuardPeerWidget::setBackground(QWidget *w, bool result) const
{
    if (result)
        w->setPalette(d->normalPalette);
    else
        w->setPalette(d->warningPalette);
}

void WireGuardPeerWidget::updatePeerWidgets()
{
    d->ui.presharedKeyLineEdit->setPasswordModeEnabled(true);

    if (d->peerData.contains(PNM_WG_PEER_KEY_PUBLIC_KEY))
        d->ui.publicKeyLineEdit->setText(d->peerData[PNM_WG_PEER_KEY_PUBLIC_KEY].toString());
    else
        d->ui.publicKeyLineEdit->clear();

    if (d->peerData.contains(PNM_WG_PEER_KEY_ALLOWED_IPS)) {
        QStringList allowedIps = d->peerData[PNM_WG_PEER_KEY_ALLOWED_IPS].toStringList();
        d->ui.allowedIPsLineEdit->setText(allowedIps.join(","));
    } else {
        d->ui.allowedIPsLineEdit->clear();
    }

    if (d->peerData.contains(PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE))
        d->ui.keepaliveLineEdit->setText(d->peerData[PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE].toString());
    else
        d->ui.keepaliveLineEdit->clear();

    // An endpoint is stored as <ipv4 | [ipv6] | fqdn>:<port>
    if (d->peerData.contains(PNM_WG_PEER_KEY_ENDPOINT)) {
        QString storedEndpoint = d->peerData[PNM_WG_PEER_KEY_ENDPOINT].toString();
        QStringList endpointList = storedEndpoint.contains("]:") ? d->peerData[PNM_WG_PEER_KEY_ENDPOINT].toString().split("]:")
                                                                 : d->peerData[PNM_WG_PEER_KEY_ENDPOINT].toString().split(":");

        d->ui.endpointAddressLineEdit->setText(endpointList[0].remove("["));
        d->ui.endpointPortLineEdit->setText(endpointList[1]);
    } else {
        d->ui.endpointAddressLineEdit->clear();
        d->ui.endpointPortLineEdit->clear();
    }
    if (d->peerData.contains(PNM_WG_PEER_KEY_PRESHARED_KEY))
        d->ui.presharedKeyLineEdit->setText(d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY].toString());
    else
        d->ui.presharedKeyLineEdit->setText("");

    if (d->peerData.contains(PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS)) {
        NetworkManager::Setting::SecretFlags type =
            static_cast<NetworkManager::Setting::SecretFlags>(d->peerData[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS].toUInt());
        switch (type) {
        case NetworkManager::Setting::AgentOwned:
            d->ui.presharedKeyLineEdit->setPasswordOption(PasswordField::StoreForUser);
            break;
        case NetworkManager::Setting::None:
            d->ui.presharedKeyLineEdit->setPasswordOption(PasswordField::StoreForAllUsers);
            break;
        // Not saved is not a valid option for the private key so set it to StoreForUser instead
        case NetworkManager::Setting::NotSaved:
            d->ui.presharedKeyLineEdit->setPasswordOption(PasswordField::StoreForUser);
            break;
        case NetworkManager::Setting::NotRequired:
            d->ui.presharedKeyLineEdit->setPasswordOption(PasswordField::NotRequired);
            break;
        }
    } else {
        d->ui.presharedKeyLineEdit->setPasswordOption(PasswordField::NotRequired);
    }

    slotWidgetChanged();
}

void WireGuardPeerWidget::slotWidgetChanged()
{
    Q_EMIT notifyValid();
}

#include "moc_wireguardpeerwidget.cpp"
