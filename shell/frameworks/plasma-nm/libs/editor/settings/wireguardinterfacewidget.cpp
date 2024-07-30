/*
    SPDX-FileCopyrightText: 2019 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "wireguardinterfacewidget.h"
#include "interfacenamevalidator.h"
#include "simpleiplistvalidator.h"
#include "wireguardkeyvalidator.h"
#include "wireguardtabwidget.h"

#include <QFile>
#include <QFileInfo>
#include <QPointer>
#include <QStandardItemModel>

#include <KColorScheme>
#include <KConfig>
#include <KConfigGroup>
#include <NetworkManagerQt/Ipv4Setting>
#include <NetworkManagerQt/Ipv6Setting>
#include <NetworkManagerQt/Utils>

#define DEFAULT_WG_INTERFACE_NAME "wg2"

// Tags used in a WireGuard .conf file - Used for importing
#define PNM_WG_CONF_TAG_INTERFACE "[Interface]"
#define PNM_WG_CONF_TAG_ADDRESS "Address"
#define PNM_WG_CONF_TAG_LISTEN_PORT "ListenPort"
#define PNM_WG_CONF_TAG_MTU "MTU"
#define PNM_WG_CONF_TAG_DNS "DNS"
#define PNM_WG_CONF_TAG_FWMARK "FwMark"
#define PNM_WG_CONF_TAG_PRIVATE_KEY "PrivateKey"
#define PNM_WG_CONF_TAG_PEER "[Peer]"
#define PNM_WG_CONF_TAG_PUBLIC_KEY "PublicKey"
#define PNM_WG_CONF_TAG_PRESHARED_KEY "PresharedKey"
#define PNM_WG_CONF_TAG_ALLOWED_IPS "AllowedIPs"
#define PNM_WG_CONF_TAG_ENDPOINT "Endpoint"
#define PNM_WG_CONF_TAG_PERSISTENT_KEEPALIVE "PersistentKeepalive"
#define PNM_WG_CONF_TAG_TABLE "Table"
#define PNM_WG_CONF_TAG_PRE_UP "PreUp"
#define PNM_WG_CONF_TAG_POST_UP "PostUp"
#define PNM_WG_CONF_TAG_PRE_DOWN "PreDown"
#define PNM_WG_CONF_TAG_POST_DOWN "PostDown"

#define PNM_WG_KEY_PEERS "peers"
#define PNM_WG_KEY_MTU "mtu"
#define PNM_WG_KEY_PEER_ROUTES "peer-routes"
#define PNM_WG_PEER_KEY_ALLOWED_IPS "allowed-ips"
#define PNM_WG_PEER_KEY_ENDPOINT "endpoint"
#define PNM_WG_PEER_KEY_PERSISTENT_KEEPALIVE "persistent-keepalive"
#define PNM_WG_PEER_KEY_PRESHARED_KEY "preshared-key"
#define PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS "preshared-key-flags"
#define PNM_WG_PEER_KEY_PUBLIC_KEY "public-key"

// Keys for the NetworkManager configuration
#define PNM_SETTING_WIREGUARD_SETTING_NAME "wireguard"

#define PNM_WG_KEY_FWMARK "fwmark"
#define PNM_WG_KEY_LISTEN_PORT "listen-port"
#define PNM_WG_KEY_PRIVATE_KEY "private-key"
#define PNM_WG_KEY_PRIVATE_KEY_FLAGS "private-key-flags"

class WireGuardInterfaceWidget::Private
{
public:
    ~Private();

    Ui_WireGuardInterfaceProp ui;
    NetworkManager::WireGuardSetting::Ptr setting;
    NetworkManager::ConnectionSettings::Ptr connectionSettings;
    KSharedConfigPtr config;
    QPalette warningPalette;
    QPalette normalPalette;
    WireGuardKeyValidator *keyValidator;
    QRegularExpressionValidator *fwmarkValidator;
    QIntValidator *mtuValidator;
    QIntValidator *portValidator;
    bool interfaceNameValid = false;
    bool privateKeyValid = false;
    bool fwmarkValid = true;
    bool listenPortValid = true;
    bool peersValid = false;
    NMVariantMapList peers;
};

WireGuardInterfaceWidget::Private::~Private()
{
    delete keyValidator;
    delete fwmarkValidator;
    delete mtuValidator;
    delete portValidator;
}

WireGuardInterfaceWidget::WireGuardInterfaceWidget(const NetworkManager::ConnectionSettings::Ptr &connectionSettings, QWidget *parent, Qt::WindowFlags f)
    : SettingWidget(connectionSettings->setting(NetworkManager::Setting::WireGuard), parent, f)
    , d(new Private)
{
    NetworkManager::Setting::Ptr setting = connectionSettings->setting(NetworkManager::Setting::WireGuard);
    d->ui.setupUi(this);
    d->connectionSettings = connectionSettings;
    d->setting = setting.staticCast<NetworkManager::WireGuardSetting>();
    d->config = KSharedConfig::openConfig();
    d->warningPalette = KColorScheme::createApplicationPalette(d->config);
    d->normalPalette = KColorScheme::createApplicationPalette(d->config);
    KColorScheme::adjustBackground(d->warningPalette, KColorScheme::NegativeBackground, QPalette::Base, KColorScheme::ColorSet::View, d->config);

    KColorScheme::adjustBackground(d->normalPalette, KColorScheme::NormalBackground, QPalette::Base, KColorScheme::ColorSet::View, d->config);

    connect(d->ui.interfaceNameLineEdit, &QLineEdit::textChanged, this, &WireGuardInterfaceWidget::checkInterfaceNameValid);
    connect(d->ui.privateKeyLineEdit, &PasswordField::textChanged, this, &WireGuardInterfaceWidget::checkPrivateKeyValid);
    connect(d->ui.privateKeyLineEdit, &PasswordField::passwordOptionChanged, this, &WireGuardInterfaceWidget::checkPrivateKeyValid);
    connect(d->ui.fwmarkLineEdit, &QLineEdit::textChanged, this, &WireGuardInterfaceWidget::checkFwmarkValid);
    connect(d->ui.listenPortLineEdit, &QLineEdit::textChanged, this, &WireGuardInterfaceWidget::checkListenPortValid);
    connect(d->ui.btnPeers, &QPushButton::clicked, this, &WireGuardInterfaceWidget::showPeers);

    d->ui.privateKeyLineEdit->setPasswordModeEnabled(true);
    d->ui.privateKeyLineEdit->setPasswordOptionsEnabled(true);
    d->ui.privateKeyLineEdit->setPasswordNotSavedEnabled(false);

    // This is done as a private variable rather than a local variable so it can be
    // used both here and to validate the private key later
    d->keyValidator = new WireGuardKeyValidator(this);

    // Test for valid interface (network device) names.
    // This test is based off of the Linux kernel's dev_valid_name.
    //   * Must be between 1-15 characters
    //   * Must be a valid filename with no whitespace or colons
    //   * Must not be "." or ".." (for simplicity, this test simply checks that the name does not start with ".")
    d->ui.interfaceNameLineEdit->setValidator(new InterfaceNameValidator(this));

    // Set default interface name
    d->ui.interfaceNameLineEdit->setText(DEFAULT_WG_INTERFACE_NAME);

    // Create validator for listen port
    d->portValidator = new QIntValidator;
    d->portValidator->setBottom(0);
    d->portValidator->setTop(65535);

    // Create a validator for the fwmark input. Acceptable
    // inputs are: "off" and numbers in either decimal
    // or hex (with 0x prefix)
    d->fwmarkValidator = new QRegularExpressionValidator(QRegularExpression("(off)|([0-9]{0,10})|(0x[0-9a-fA-F]{1,8})"));
    d->ui.fwmarkLineEdit->setValidator(d->fwmarkValidator);

    // Create a validator for the MTU field.
    d->mtuValidator = new QIntValidator();
    d->mtuValidator->setBottom(0);
    d->ui.mtuLineEdit->setValidator(d->mtuValidator);

    // Default Peer Routes to true
    d->ui.peerRouteCheckBox->setChecked(true);

    // Connect for setting check
    watchChangedSetting();

    KAcceleratorManager::manage(this);

    if (setting && !setting->isNull()) {
        loadConfig(d->setting);
    }

    // Set the initial backgrounds on all the widgets
    checkPrivateKeyValid();
    checkInterfaceNameValid();
}

WireGuardInterfaceWidget::~WireGuardInterfaceWidget()
{
    delete d;
}

void WireGuardInterfaceWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    if (d->connectionSettings != nullptr) {
        d->ui.interfaceNameLineEdit->setText(d->connectionSettings->interfaceName());
    }

    NetworkManager::WireGuardSetting::Ptr wireGuardSetting = setting.staticCast<NetworkManager::WireGuardSetting>();
    d->ui.privateKeyLineEdit->setText(wireGuardSetting->privateKey());

    if (wireGuardSetting->listenPort() != 0)
        d->ui.listenPortLineEdit->setText(QString::number(wireGuardSetting->listenPort()));
    else
        d->ui.listenPortLineEdit->clear();

    if (wireGuardSetting->fwmark() != 0)
        d->ui.fwmarkLineEdit->setText(QString::number(wireGuardSetting->fwmark()));
    else
        d->ui.fwmarkLineEdit->clear();

    if (wireGuardSetting->mtu() != 0)
        d->ui.mtuLineEdit->setText(QString::number(wireGuardSetting->mtu()));
    else
        d->ui.mtuLineEdit->clear();

    d->ui.peerRouteCheckBox->setChecked(wireGuardSetting->peerRoutes());

    NetworkManager::Setting::SecretFlags type = wireGuardSetting->privateKeyFlags();
    switch (type) {
    case NetworkManager::Setting::AgentOwned:
        d->ui.privateKeyLineEdit->setPasswordOption(PasswordField::StoreForUser);
        break;
    case NetworkManager::Setting::None:
        d->ui.privateKeyLineEdit->setPasswordOption(PasswordField::StoreForAllUsers);
        break;
    // Not saved is not a valid option for the private key so set it to StoreForUser instead
    case NetworkManager::Setting::NotSaved:
        d->ui.privateKeyLineEdit->setPasswordOption(PasswordField::StoreForUser);
        break;
    case NetworkManager::Setting::NotRequired:
        d->ui.privateKeyLineEdit->setPasswordOption(PasswordField::NotRequired);
        break;
    }

    d->peers = wireGuardSetting->peers();
    loadSecrets(setting);
}

void WireGuardInterfaceWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    NetworkManager::WireGuardSetting::Ptr wireGuardSetting = setting.staticCast<NetworkManager::WireGuardSetting>();
    if (wireGuardSetting) {
        const QString key = wireGuardSetting->privateKey();
        if (!key.isEmpty())
            d->ui.privateKeyLineEdit->setText(key);

        const NMVariantMapList peers = wireGuardSetting->peers();
        if (!peers.isEmpty()) {
            // For each of the peers returned, see if it contains a preshared key
            for (QList<QVariantMap>::const_iterator i = peers.cbegin(); i != peers.cend(); i++) {
                if (i->contains(PNM_WG_PEER_KEY_PRESHARED_KEY)) {
                    // We have a preshared key so find the matching public key in the local peer list
                    QString currentPublicKey = (*i)[PNM_WG_PEER_KEY_PUBLIC_KEY].toString();
                    if (!currentPublicKey.isEmpty()) {
                        for (QList<QVariantMap>::iterator j = d->peers.begin(); j != d->peers.end(); j++) {
                            if ((*j)[PNM_WG_PEER_KEY_PUBLIC_KEY].toString() == currentPublicKey) {
                                (*j)[PNM_WG_PEER_KEY_PRESHARED_KEY] = (*i)[PNM_WG_PEER_KEY_PRESHARED_KEY].toString();
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    // On the assumption that a saved configuration is valid (because how could it be saved if it
    // wasn't valid) then the peers section is valid unless it didn't get a preshared key if one
    // is required, so a simple minded "validity" check is done here. Real validity checks are done
    // when the peers widget is open which is the only time changes which might be invalid are
    // possible.
    d->peersValid = true;
    for (QList<QVariantMap>::iterator j = d->peers.begin(); j != d->peers.end(); j++) {
        if ((*j).contains(PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS) && (*j)[PNM_WG_PEER_KEY_PRESHARED_KEY_FLAGS] != NetworkManager::Setting::NotRequired
            && (!(*j).contains(PNM_WG_PEER_KEY_PRESHARED_KEY) || (*j)[PNM_WG_PEER_KEY_PRESHARED_KEY].toString().isEmpty())) {
            d->peersValid = false;
            break;
        }
    }
}

QVariantMap WireGuardInterfaceWidget::setting() const
{
    NetworkManager::WireGuardSetting wgSetting;
    QString val = d->ui.fwmarkLineEdit->displayText();

    if (!val.isEmpty())
        wgSetting.setFwmark(val.toUInt());

    val = d->ui.listenPortLineEdit->displayText();
    if (!val.isEmpty())
        wgSetting.setListenPort(val.toUInt());

    val = d->ui.mtuLineEdit->displayText();
    if (!val.isEmpty())
        wgSetting.setMtu(val.toUInt());

    val = d->ui.privateKeyLineEdit->text();
    if (!val.isEmpty())
        wgSetting.setPrivateKey(val);

    wgSetting.setPeerRoutes(d->ui.peerRouteCheckBox->isChecked());

    PasswordField::PasswordOption option = d->ui.privateKeyLineEdit->passwordOption();
    switch (option) {
    case PasswordField::StoreForUser:
        wgSetting.setPrivateKeyFlags(NetworkManager::Setting::AgentOwned);
        break;
    case PasswordField::StoreForAllUsers:
        wgSetting.setPrivateKeyFlags(NetworkManager::Setting::None);
        break;
    // Always Ask is not a valid option for the private key so set it to AgentOwned instead
    case PasswordField::AlwaysAsk:
        wgSetting.setPrivateKeyFlags(NetworkManager::Setting::AgentOwned);
        break;
    case PasswordField::NotRequired:
        wgSetting.setPrivateKeyFlags(NetworkManager::Setting::NotRequired);
        break;
    }
    wgSetting.setPeers(d->peers);
    return wgSetting.toMap();
}

QString WireGuardInterfaceWidget::interfaceName() const
{
    return d->ui.interfaceNameLineEdit->displayText();
}

bool WireGuardInterfaceWidget::isValid() const
{
    return d->interfaceNameValid && d->privateKeyValid && d->fwmarkValid && d->listenPortValid && d->peersValid;
}

void WireGuardInterfaceWidget::checkInterfaceNameValid()
{
    int pos = 0;
    QLineEdit *widget = d->ui.interfaceNameLineEdit;
    QString value = widget->displayText();
    d->interfaceNameValid = (QValidator::Acceptable == widget->validator()->validate(value, pos));
    setBackground(widget, d->interfaceNameValid);
    slotWidgetChanged();
}
void WireGuardInterfaceWidget::checkPrivateKeyValid()
{
    int pos = 0;
    PasswordField *widget = d->ui.privateKeyLineEdit;
    QString value = widget->text();
    bool valid = (QValidator::Acceptable == d->keyValidator->validate(value, pos));
    d->privateKeyValid = valid;
    setBackground(widget, valid);
    slotWidgetChanged();
}
void WireGuardInterfaceWidget::checkFwmarkValid()
{
    int pos = 0;
    QLineEdit *widget = d->ui.fwmarkLineEdit;
    QString value = widget->displayText();
    d->fwmarkValid = QValidator::Acceptable == widget->validator()->validate(value, pos) || value.isEmpty();
    setBackground(widget, d->fwmarkValid);
    slotWidgetChanged();
}

void WireGuardInterfaceWidget::checkListenPortValid()
{
    int pos = 0;
    QLineEdit *widget = d->ui.listenPortLineEdit;
    QString value = widget->displayText();
    d->listenPortValid = QValidator::Acceptable == d->portValidator->validate(value, pos) || value.isEmpty();
    setBackground(widget, d->listenPortValid);
    slotWidgetChanged();
}

void WireGuardInterfaceWidget::setBackground(QWidget *w, bool result) const
{
    if (result)
        w->setPalette(d->normalPalette);
    else
        w->setPalette(d->warningPalette);
}

QString WireGuardInterfaceWidget::supportedFileExtensions()
{
    return "*.conf";
}

void WireGuardInterfaceWidget::showPeers()
{
    QPointer<WireGuardTabWidget> peers = new WireGuardTabWidget(d->peers, this);
    peers->setAttribute(Qt::WA_DeleteOnClose);

    connect(peers.data(), &WireGuardTabWidget::accepted, [peers, this]() {
        NMVariantMapList peersData = peers->setting();
        if (!peersData.isEmpty()) {
            d->peers = peersData;
            // The peers widget won't allow "OK" to be hit
            // unless all the peers are valid so no need to check
            d->peersValid = true;
            slotWidgetChanged();
        }
    });
    peers->setModal(true);
    peers->show();
}

// Take any input string and turn it into a valid interface name.
QString WireGuardInterfaceWidget::sanitizeInterfaceName(const QString &interfaceName)
{
    QString ret(interfaceName);

    // Remove all whitespace, slash, and colon characters
    ret.removeIf([](QChar c) {
        return c == '\r' || c == '\n' || c == '\t' || c == '\f' || c == ' ' || c == ':' || c == '/';
    });

    // Iterface names on Linux must be < 16 chars
    if (ret.length() >= 16) {
        ret.remove(15, ret.length() - 15);
    }

    if (ret.length() == 0 || ret == "." || ret == "..") {
        return DEFAULT_WG_INTERFACE_NAME; // Fall-back to a sane default
    }

    return ret;
}

NMVariantMapMap WireGuardInterfaceWidget::importConnectionSettings(const QString &fileName)
{
    NMVariantMapMap result;

    QFile impFile(fileName);
    QList<NetworkManager::IpAddress> ipv4AddressList;
    QList<NetworkManager::IpAddress> ipv6AddressList;

    if (!impFile.open(QFile::ReadOnly | QFile::Text)) {
        return result;
    }

    const QString connectionName = QFileInfo(fileName).completeBaseName();
    const QString interfaceName = sanitizeInterfaceName(connectionName);
    NMVariantMapList peers;
    QVariantMap *currentPeer = nullptr;
    WireGuardKeyValidator keyValidator;
    NetworkManager::Ipv4Setting ipv4Setting;
    NetworkManager::Ipv6Setting ipv6Setting;
    NetworkManager::WireGuardSetting wgSetting;

    bool havePrivateKey = false;
    bool haveIpv4Setting = false;
    bool haveIpv6Setting = false;
    // Set the "PEER" elements true because they
    // need to be true to allocate the
    // first [Peer] section below
    bool havePublicKey = true;
    bool haveAllowedIps = true;
    int pos = 0;

    QTextStream in(&impFile);
    enum { IDLE, INTERFACE_SECTION, PEER_SECTION } currentState = IDLE;

    ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Disabled);
    ipv6Setting.setMethod(NetworkManager::Ipv6Setting::Ignored);

    while (!in.atEnd()) {
        QStringList keyValue;
        QString line = in.readLine();

        // Remove comments starting with '#'
        if (int index = line.indexOf(QLatin1Char('#')) > 0)
            line.truncate(index);

        // Ignore blank lines
        if (line.isEmpty())
            continue;

        keyValue.clear();
        keyValue << line.split(QLatin1Char('='));

        if (keyValue[0] == PNM_WG_CONF_TAG_INTERFACE) {
            currentState = INTERFACE_SECTION;
            continue;
        } else if (keyValue[0] == PNM_WG_CONF_TAG_PEER) {
            // Check to make sure the previous PEER section has
            // all the required elements. If not it's an error
            // so just return the empty result.
            if (!havePublicKey || !haveAllowedIps) {
                return result;
            } else {
                havePublicKey = false;
                haveAllowedIps = false;
                currentState = PEER_SECTION;
                peers.append(*(new QVariantMap));
                currentPeer = &peers[peers.size() - 1];
                continue;
            }
        }

        // If we didn't get an '=' sign in the line, it's probably an error but
        // we're going to treat it as a comment and ignore it
        if (keyValue.length() < 2)
            continue;

        QString key = keyValue[0].trimmed();

        // If we are in the [Interface] section look for the possible tags
        if (currentState == INTERFACE_SECTION) {
            // Address
            if (key == PNM_WG_CONF_TAG_ADDRESS) {
                QStringList valueList = keyValue[1].split(QLatin1Char(','));
                if (valueList.isEmpty())
                    return result;

                for (const QString &address : valueList) {
                    const QPair<QHostAddress, int> addressIn = QHostAddress::parseSubnet(address.trimmed());
                    auto addr = new NetworkManager::IpAddress;
                    addr->setIp(addressIn.first);
                    addr->setPrefixLength(addressIn.second);
                    if (addressIn.first.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
                        ipv4AddressList.append(*addr);
                    } else if (addressIn.first.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv6Protocol) {
                        ipv6AddressList.append(*addr);
                    } else { // Error condition
                        return result;
                    }
                }
                if (!ipv4AddressList.isEmpty()) {
                    ipv4Setting.setAddresses(ipv4AddressList);
                    ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Manual);
                    haveIpv4Setting = true;
                }
                if (!ipv6AddressList.isEmpty()) {
                    ipv6Setting.setAddresses(ipv6AddressList);
                    ipv6Setting.setMethod(NetworkManager::Ipv6Setting::Manual);
                    haveIpv6Setting = true;
                }
            }

            // Listen Port
            else if (key == PNM_WG_CONF_TAG_LISTEN_PORT) {
                uint val = keyValue[1].toUInt();
                if (val <= 65535)
                    wgSetting.setListenPort(val);
            } else if (key == PNM_WG_CONF_TAG_PRIVATE_KEY) {
                QString val = keyValue[1].trimmed();
                // WireGuard keys present a slight problem because they
                // must end in '=' which is removed during the 'split' above
                // so if there are 3 parts and the 3 is empty, there must
                // have been an '=' so add it back on
                if (keyValue.size() == 3 && keyValue[2].isEmpty())
                    val += "=";
                if (QValidator::Acceptable == keyValidator.validate(val, pos)) {
                    wgSetting.setPrivateKey(val);
                    havePrivateKey = true;
                }
            } else if (key == PNM_WG_CONF_TAG_DNS) {
                QStringList addressList = keyValue[1].split(QLatin1Char(','));
                QList<QHostAddress> ipv4DnsList;
                QList<QHostAddress> ipv6DnsList;
                if (!addressList.isEmpty()) {
                    for (const QString &address : addressList) {
                        const QPair<QHostAddress, int> addressIn = QHostAddress::parseSubnet(address.trimmed());
                        if (addressIn.first.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol) {
                            ipv4DnsList.append(addressIn.first);
                        } else if (addressIn.first.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv6Protocol) {
                            ipv6DnsList.append(addressIn.first);
                        } else { // Error condition
                            return result;
                        }
                    }
                }

                // If there are any addresses put them on the correct tab and set the "method" to
                // "Automatic (Only addresses)" by setting "ignore-auto-dns=true"
                if (!ipv4DnsList.isEmpty()) {
                    if (ipv4Setting.method() == NetworkManager::Ipv4Setting::Disabled)
                        ipv4Setting.setMethod(NetworkManager::Ipv4Setting::Automatic);
                    ipv4Setting.setIgnoreAutoDns(true);
                    ipv4Setting.setDns(ipv4DnsList);
                    haveIpv4Setting = true;
                }

                if (!ipv6DnsList.isEmpty()) {
                    ipv6Setting.setMethod(NetworkManager::Ipv6Setting::Automatic);
                    ipv6Setting.setIgnoreAutoDns(true);
                    ipv6Setting.setDns(ipv6DnsList);
                    haveIpv6Setting = true;
                }
            } else if (key == PNM_WG_CONF_TAG_MTU) {
                uint val = keyValue[1].toUInt();
                if (val > 0)
                    wgSetting.setMtu(val);
            } else if (key == PNM_WG_CONF_TAG_FWMARK) {
                uint val;
                if (keyValue[1].trimmed().toLower() == QLatin1String("off"))
                    val = 0;
                else
                    val = keyValue[1].toUInt();
                wgSetting.setFwmark(val);
            } else if (key == PNM_WG_CONF_TAG_TABLE //
                       || key == PNM_WG_CONF_TAG_PRE_UP //
                       || key == PNM_WG_CONF_TAG_POST_UP //
                       || key == PNM_WG_CONF_TAG_PRE_DOWN //
                       || key == PNM_WG_CONF_TAG_POST_DOWN) {
                // plasma-nm does not handle these items
            } else {
                // We got a wrong field in the Interface section so it
                // is an error
                break;
            }
        } else if (currentState == PEER_SECTION) {
            // Public Key
            if (key == PNM_WG_CONF_TAG_PUBLIC_KEY) {
                QString val = keyValue[1].trimmed();
                // WireGuard keys present a slight problem because they
                // must end in '=' which is removed during the 'split' above
                // so if there are 3 parts and the 3 is empty, there must
                // have been an '=' so add it back on
                if (keyValue.size() == 3 && keyValue[2].isEmpty())
                    val += "=";

                if (QValidator::Acceptable == keyValidator.validate(val, pos)) {
                    currentPeer->insert(PNM_WG_PEER_KEY_PUBLIC_KEY, val);
                    havePublicKey = true;
                }
            } else if (key == PNM_WG_CONF_TAG_ALLOWED_IPS) {
                SimpleIpListValidator validator(SimpleIpListValidator::WithCidr, SimpleIpListValidator::Both);
                QString val = keyValue[1].trimmed();
                if (QValidator::Acceptable == validator.validate(val, pos)) {
                    QStringList valList = val.split(QLatin1Char(','));
                    for (QString &str : valList)
                        str = str.trimmed();
                    currentPeer->insert(PNM_WG_PEER_KEY_ALLOWED_IPS, valList);
                    haveAllowedIps = true;
                }
            } else if (key == PNM_WG_CONF_TAG_ENDPOINT) {
                if (keyValue[1].length() > 0)
                    currentPeer->insert(PNM_WG_PEER_KEY_ENDPOINT, keyValue[1].trimmed());
            } else if (key == PNM_WG_CONF_TAG_PRESHARED_KEY) {
                QString val = keyValue[1].trimmed();
                // WireGuard keys present a slight problem because they
                // must end in '=' which is removed during the 'split' above
                // so if there are 3 parts and the 3 is empty, there must
                // have been an '=' so add it back on
                if (keyValue.size() == 3 && keyValue[2].isEmpty())
                    val += "=";

                if (QValidator::Acceptable == keyValidator.validate(val, pos)) {
                    currentPeer->insert(PNM_WG_PEER_KEY_PRESHARED_KEY, val);
                }
            }
        } else {
            return result;
        }
    }
    if (!havePrivateKey || !haveAllowedIps || !havePublicKey)
        return result;

    QVariantMap conn;
    wgSetting.setPeers(peers);
    conn.insert("id", connectionName);
    conn.insert("interface-name", interfaceName);
    conn.insert("type", "wireguard");
    conn.insert("autoconnect", "false");
    result.insert("connection", conn);
    result.insert("wireguard", wgSetting.toMap());
    if (haveIpv4Setting)
        result.insert("ipv4", ipv4Setting.toMap());
    if (haveIpv6Setting)
        result.insert("ipv6", ipv6Setting.toMap());

    impFile.close();
    return result;
}

#include "moc_wireguardinterfacewidget.cpp"
