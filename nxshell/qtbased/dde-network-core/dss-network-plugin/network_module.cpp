// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "network_module.h"
#include "networkpluginhelper.h"
#include "networkdialog.h"
#include "secretagent.h"
#include "notificationmanager.h"
#include "networkdialog/thememanager.h"

#include <QWidget>
#include <QTime>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QLabel>

#include <DArrowRectangle>
#include <DIconButton>

#include <networkcontroller.h>
#include <networkdevicebase.h>

#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WiredDevice>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/WiredSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

#define NETWORK_KEY "network-item-key"

using namespace NetworkManager;

DWIDGET_USE_NAMESPACE
NETWORKPLUGIN_USE_NAMESPACE

namespace dss {
namespace module {

NetworkModule::NetworkModule(QObject *parent)
    : QObject(parent)
    , m_lastState(NetworkManager::Device::State::UnknownState)
    , m_initilized(false)
{
    QDBusConnection::sessionBus().connect("org.deepin.dde.LockFront1", "/org/deepin/dde/LockFront1", "org.deepin.dde.LockFront1", "Visible", this, SLOT(updateLockScreenStatus(bool)));
    m_isLockModel = (-1 == qAppName().indexOf("greeter"));
    if (!m_isLockModel) {
        dde::network::NetworkController::setServiceType(dde::network::ServiceLoadType::LoadFromManager);
    }

    m_networkDialog = new NetworkDialog(this);
    m_networkHelper = new NetworkPluginHelper(m_networkDialog, this);
    m_panelContainer = new NetworkPanelContainer(m_networkDialog);
    connect(m_networkHelper, &NetworkPluginHelper::pluginStateChanged, m_panelContainer, &NetworkPanelContainer::onPluginStateChanged);
    m_panelContainer->onPluginStateChanged(m_networkHelper->getPluginState());

    installTranslator(QLocale::system().name());
    ThemeManager::ref().setThemeType(m_isLockModel ? ThemeManager::LockType : ThemeManager::GreeterType);
    if (m_isLockModel) {
        m_networkDialog->setServerName("dde-network-dialog" + QString::number(getuid()) + "lock");
    } else {
        QDBusMessage lock = QDBusMessage::createMethodCall("org.deepin.dde.LockService1", "/org/deepin/dde/LockService1", "org.deepin.dde.LockService1", "CurrentUser");
        QDBusConnection::systemBus().callWithCallback(lock, this, SLOT(onUserChanged(QString)));
        QDBusConnection::systemBus().connect("org.deepin.dde.LockService1", "/org/deepin/dde/LockService1", "org.deepin.dde.LockService1", "UserChanged", this, SLOT(onUserChanged(QString)));

        m_secretAgent = new NETWORKPLUGIN_NAMESPACE::SecretAgent(true, this);
        connect(m_networkDialog, &NetworkDialog::inputPassword, m_secretAgent, &NETWORKPLUGIN_NAMESPACE::SecretAgent::onInputPassword);
        connect(m_secretAgent, &NETWORKPLUGIN_NAMESPACE::SecretAgent::requestPassword, m_networkDialog, &NetworkDialog::setConnectWireless);
    }
}

NetworkModule::~NetworkModule()
{
    if (m_panelContainer) {
        m_panelContainer->deleteLater();
        m_panelContainer.clear();
    }
}

QWidget *NetworkModule::content()
{
    return m_panelContainer;
}

QWidget *NetworkModule::itemTipsWidget() const
{
    QWidget *itemTips = m_networkHelper->itemTips();
    if (itemTips) {
        QPalette palette = itemTips->palette();
        palette.setColor(QPalette::BrightText, Qt::white);
        itemTips->setPalette(palette);
    }
    return itemTips;
}

const QString NetworkModule::itemContextMenu() const
{
    return m_networkHelper->contextMenu(false);
}

void NetworkModule::invokedMenuItem(const QString &menuId, const bool checked) const
{
    Q_UNUSED(checked);
    m_networkHelper->invokeMenuItem(menuId);
}

void NetworkModule::updateLockScreenStatus(bool visible)
{
    m_isLockModel = true;
    m_isLockScreen = visible;
}

void NetworkModule::onAddDevice(const QString &devicePath)
{
    if (m_isLockModel) {
        return;
    }
    // 登录界面才监听该信号，用于横幅、密码错误处理
    if (!m_devicePaths.contains(devicePath)) {
        Device::Ptr device(new Device(devicePath));
        Device *nmDevice = nullptr;
        if (device->type() == Device::Wifi) {
            NetworkManager::WirelessDevice *wDevice = new NetworkManager::WirelessDevice(devicePath, this);
            nmDevice = wDevice;
            connect(wDevice, &NetworkManager::WirelessDevice::activeAccessPointChanged, this, [this](const QString &ap) {
                m_lastActiveWirelessDevicePath = static_cast<NetworkManager::WirelessDevice *>(sender())->uni() + NetworkManager::AccessPoint(ap).ssid();
            });
        } else if (device->type() == Device::Ethernet) {
            NetworkManager::WiredDevice *wDevice = new NetworkManager::WiredDevice(devicePath, this);
            nmDevice = wDevice;
            addFirstConnection(wDevice);
        }
        if (nmDevice) {
            connect(nmDevice, &NetworkManager::Device::stateChanged, this, &NetworkModule::onDeviceStatusChanged);
            m_devicePaths.insert(devicePath);
        }
    }
}

void NetworkModule::onUserChanged(QString json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isObject()) {
        int uid = doc.object().value("Uid").toInt();
        QDBusInterface user("org.deepin.dde.Accounts1", QString("/org/deepin/dde/Accounts1/User%1").arg(uid), "org.deepin.dde.Accounts1.User", QDBusConnection::systemBus());
        installTranslator(user.property("Locale").toString().split(".").first());
    }
    if (!m_initilized) {
        m_initilized = true;
        connect(m_networkHelper, &NetworkPluginHelper::addDevice, this, &NetworkModule::onAddDevice);
        for (dde::network::NetworkDeviceBase *device : dde::network::NetworkController::instance()->devices()) {
            onAddDevice(device->path());
        }
    }
}

void NetworkModule::installTranslator(QString locale)
{
    static QTranslator translator;
    static QString localTmp;
    if (localTmp == locale) {
        return;
    }
    localTmp = locale;
    QApplication::removeTranslator(&translator);
    QString languagePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                  QString("dss-network-plugin/translations"),
                                                  QStandardPaths::LocateDirectory);
    translator.load(QString(languagePath+"/dss-network-plugin_%1.qm").arg(locale));
    QApplication::installTranslator(&translator);
    dde::network::NetworkController::instance()->retranslate();
    m_networkHelper->updateTooltips();
    m_panelContainer->onPluginStateChanged(m_networkHelper->getPluginState());
}

const QString NetworkModule::connectionMatchName() const
{
    NetworkManager::Connection::List connList = listConnections();
    QStringList connNameList;
    int connSuffixNum = 1;

    for (NetworkManager::Connection::Ptr conn : connList) {
        if (conn->settings()->connectionType() == ConnectionSettings::ConnectionType::Wired)
            connNameList.append(conn->name());
    }

    QString matchConnName = QString(tr("Wired Connection")) + QString(" %1");
    for (int i = 1; i <= connNameList.size(); ++i) {
        if (!connNameList.contains(matchConnName.arg(i))) {
            connSuffixNum = i;
            break;
        }
        if (i == connNameList.size())
            connSuffixNum = i + 1;
    }

    return matchConnName.arg(connSuffixNum);
}

bool NetworkModule::hasConnection(NetworkManager::WiredDevice *nmDevice, NetworkManager::Connection::List &unSaveDevices)
{
    bool connIsEmpty = false;
    // 获取所有的连接列表,遍历连接列表,如果当前连接中的MAC地址不为空且不等于当前网卡的MAC地址，则认为它不是当前网卡的连接
    // 在获取所有连接之前，需要手动调用一下当前设备的availableConnections接口，否则获取到的连接列表就不是最新的(具体原因待查)。
    nmDevice->availableConnections();
    NetworkManager::Connection::List connList = listConnections();
    for (NetworkManager::Connection::Ptr conn : connList) {
        WiredSetting::Ptr settings = conn->settings()->setting(Setting::Wired).staticCast<WiredSetting>();
        // 如果当前连接的MAC地址不为空且连接的MAC地址不等于当前设备的MAC地址，则认为不是当前的连接，跳过
        if (settings.isNull() ||
            (!settings->macAddress().isEmpty() && nmDevice->hardwareAddress().compare(settings->macAddress().toHex(':'), Qt::CaseInsensitive) != 0) ||
            (conn->settings()->interfaceName() !=  nmDevice->interfaceName()))
            continue;

        // 将未保存的连接放入到列表中，供外面调用删除
        if (conn->isUnsaved()) {
            unSaveDevices << conn;
            continue;
        }

        connIsEmpty = true;
    }

    return connIsEmpty;
}

void NetworkModule::addFirstConnection(NetworkManager::WiredDevice *nmDevice)
{
    // 先查找当前的设备下是否存在有线连接，如果不存在，则直接新建一个，因为按照要求是至少要有一个有线连接
    NetworkManager::Connection::List unSaveConnections;
    bool findConnection = hasConnection(nmDevice, unSaveConnections);
    // 按照需求，需要将未保存的连接删除
    bool isRemoved = !unSaveConnections.isEmpty();
    for (NetworkManager::Connection::Ptr conn : unSaveConnections)
        conn->remove();

    auto autoCreateConnection = [this, nmDevice]() {
        // 如果发现当前的连接的数量为空,则自动创建以当前语言为基础的连接
        ConnectionSettings::Ptr conn(new ConnectionSettings);
        conn->setId(connectionMatchName());
        conn->setInterfaceName(nmDevice->interfaceName());
        conn->setUuid("");
        NetworkManager::addConnection(conn->toMap());
    };

    if (!findConnection) {
        if (isRemoved) {
            // 如果有删除的连接，则等待1秒后重新创建
            QTimer::singleShot(1000, this, [autoCreateConnection] {
                autoCreateConnection();
            });
        } else {
            autoCreateConnection();
        }
    }
}

bool NetworkModule::needPopupNetworkDialog() const
{
    // 如果上一次没有保存密码信息则不弹窗
    if (m_lastConnectionUuid.isEmpty())
        return false;

    NetworkManager::Connection::Ptr connection = NetworkManager::findConnectionByUuid(m_lastConnectionUuid);
    if (connection.isNull())
        return false;

    // 如果当前连接的密码是按照用户保存的，就不弹出来
    WirelessSecuritySetting::Ptr securitySetting = connection->settings()->setting(Setting::SettingType::WirelessSecurity).staticCast<WirelessSecuritySetting>();
    if (securitySetting.isNull())
        return true;

    NetworkManager::Setting::SecretFlags passwordFlags = securitySetting->pskFlags();
    return (passwordFlags.testFlag(Setting::None));
}

void NetworkModule::onDeviceStatusChanged(NetworkManager::Device::State newstate, NetworkManager::Device::State oldstate, NetworkManager::Device::StateChangeReason reason)
{
    if (m_isLockModel) {
        return;
    }
    NetworkManager::Device *device = static_cast<NetworkManager::Device *>(sender());
    NetworkManager::ActiveConnection::Ptr conn = device->activeConnection();
    if (!conn.isNull()) {
        m_lastConnection = conn->id();
        m_lastConnectionUuid = conn->uuid();
        m_lastState = newstate;
    } else if (m_lastState != oldstate || m_lastConnection.isEmpty()) {
        m_lastConnection.clear();
        m_lastConnectionUuid.clear();
        return;
    }
    switch (newstate) {
    case Device::State::Preparing: { // 正在连接
        if (oldstate == Device::State::Disconnected) {
            switch (device->type()) {
            case Device::Type::Ethernet:
                NotificationManager::NetworkNotify(NotificationManager::WiredConnecting, m_lastConnection);
                break;
            case Device::Type::Wifi:
                NotificationManager::NetworkNotify(NotificationManager::WirelessConnecting, m_lastConnection);
                break;
            default:
                break;
            }
        }
    } break;
    case Device::State::Activated: { // 连接成功
        switch (device->type()) {
        case Device::Type::Ethernet:
            NotificationManager::NetworkNotify(NotificationManager::WiredConnected, m_lastConnection);
            break;
        case Device::Type::Wifi:
            NotificationManager::NetworkNotify(NotificationManager::WirelessConnected, m_lastConnection);
            break;
        default:
            break;
        }
    } break;
    case Device::State::Failed:
    case Device::State::Disconnected:
    case Device::State::NeedAuth:
    case Device::State::Unmanaged:
    case Device::State::Unavailable: {
        if (reason == Device::StateChangeReason::DeviceRemovedReason) {
            return;
        }

        // ignore if device's old state is not available
        if (oldstate <= Device::State::Unavailable) {
            qDebug("no notify, old state is not available");
            return;
        }

        // ignore invalid reasons
        if (reason == Device::StateChangeReason::UnknownReason) {
            qDebug("no notify, device state reason invalid");
            return;
        }

        switch (reason) {
        case Device::StateChangeReason::UserRequestedReason:
            if (newstate == Device::State::Disconnected) {
                switch (device->type()) {
                case Device::Type::Ethernet:
                    NotificationManager::NetworkNotify(NotificationManager::WiredDisconnected, m_lastConnection);
                    break;
                case Device::Type::Wifi:
                    NotificationManager::NetworkNotify(NotificationManager::WirelessDisconnected, m_lastConnection);
                    break;
                default:
                    break;
                }
            }
            break;
        case Device::StateChangeReason::ConfigUnavailableReason:
        case Device::StateChangeReason::AuthSupplicantTimeoutReason: // 超时
            switch (device->type()) {
            case Device::Type::Ethernet:
                NotificationManager::NetworkNotify(NotificationManager::WiredUnableConnect, m_lastConnection);
                break;
            case Device::Type::Wifi:
                NotificationManager::NetworkNotify(NotificationManager::WirelessUnableConnect, m_lastConnection);
                break;
            default:
                break;
            }
            break;
        case Device::StateChangeReason::AuthSupplicantDisconnectReason:
            if (oldstate == Device::State::ConfiguringHardware && newstate == Device::State::NeedAuth) {
                switch (device->type()) {
                case Device::Type::Ethernet:
                    NotificationManager::NetworkNotify(NotificationManager::WiredConnectionFailed, m_lastConnection);
                    break;
                case Device::Type::Wifi:
                    NotificationManager::NetworkNotify(NotificationManager::WirelessConnectionFailed, m_lastConnection);
                    if (needPopupNetworkDialog()) {
                        m_networkDialog->setConnectWireless(device->uni(), m_lastConnection);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case Device::StateChangeReason::CarrierReason:
            if (device->type() == Device::Ethernet) {
                qDebug("unplugged device is ethernet");
                NotificationManager::NetworkNotify(NotificationManager::WiredDisconnected, m_lastConnection);
            }
            break;
        case Device::StateChangeReason::NoSecretsReason:
            NotificationManager::NetworkNotify(NotificationManager::NoSecrets, m_lastConnection);
            if (needPopupNetworkDialog()) {
                // 不是仅当前用户，就弹窗
                m_networkDialog->setConnectWireless(device->uni(), m_lastConnection);
            }
            break;
        case Device::StateChangeReason::SsidNotFound:
            NotificationManager::NetworkNotify(NotificationManager::SsidNotFound, m_lastConnection);
            break;
        default:
            break;
        }
    } break;
    default:
        break;
    }
}

NetworkPlugin::NetworkPlugin(QObject *parent)
    : QObject(parent)
    , m_network(nullptr)
{
    setObjectName(QStringLiteral(NETWORK_KEY));
}

void NetworkPlugin::init()
{
}

QWidget *NetworkPlugin::content()
{
    ensureNetwork();
    return m_network->content();
}

QString NetworkPlugin::icon() const
{
    const_cast<NetworkPlugin *>(this)->ensureNetwork();
    return m_network->networkHelper()->iconPath(DGuiApplicationHelper::instance()->themeType());
}

QWidget *NetworkPlugin::itemWidget() const
{
    DIconButton *iconButton = new DIconButton;
    iconButton->setFocusPolicy(Qt::NoFocus);
    iconButton->setFlat(true);
    iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    iconButton->setIconSize({26, 26});
    QTimer::singleShot(1, [this, iconButton]() {
        const_cast<NetworkPlugin *>(this)->ensureNetwork();
        auto helper = m_network->networkHelper();
        iconButton->setIcon(*helper->trayIcon());
        connect(helper, &NetworkPluginHelper::iconChanged, iconButton, [helper, iconButton] {
            iconButton->setIcon(*helper->trayIcon());
        });
    });
    NotificationManager::InstallEventFilter(iconButton);
    return iconButton;
}

QWidget *NetworkPlugin::itemTipsWidget() const
{
    const_cast<NetworkPlugin *>(this)->ensureNetwork();
    return m_network->itemTipsWidget();
}

const QString NetworkPlugin::itemContextMenu() const
{
    const_cast<NetworkPlugin *>(this)->ensureNetwork();
    return m_network->itemContextMenu();
}

void NetworkPlugin::invokedMenuItem(const QString &menuId, const bool checked) const
{
    const_cast<NetworkPlugin *>(this)->ensureNetwork();
    m_network->invokedMenuItem(menuId, checked);
}

// Delay construct Network to avoid NetworkManager service hasn't ready, e.g: dde-session-daemon.
void NetworkPlugin::ensureNetwork()
{
    if (m_network) {
        return;
    }

    m_network = new NetworkModule(this);
}

NetworkPanelContainer::NetworkPanelContainer(dde::networkplugin::NetworkDialog *dialog, QWidget *parent)
    : QWidget(parent)
    , m_warnLabel(new QLabel)
    , m_dialog(dialog)
    , m_savedParent(dialog->panel()->parentWidget())
    , m_contentWidget(nullptr)
{
    m_warnLabel->setFixedWidth(300);
    m_warnLabel->setAlignment(Qt::AlignCenter);
    m_warnLabel->setContentsMargins(10, 0, 10, 0);
    m_warnLabel->setWordWrap(true);
}

NetworkPanelContainer::~NetworkPanelContainer()
{
    // Warning label may have a null parent, delete it here.
    delete m_warnLabel;
    m_warnLabel = nullptr;
}

void NetworkPanelContainer::sendWarnMessage(const QString &msg)
{
    m_warnLabel->setText(msg);
    setContentWidget(m_warnLabel);
}

void NetworkPanelContainer::clearWarnMessage()
{
    m_warnLabel->clear();
    setContentWidget(m_dialog->panel());
}

void NetworkPanelContainer::onPluginStateChanged(PluginState state)
{
    switch(state) {
    case PluginState::Nocable:
    case PluginState::Unknown:
    case PluginState::Disabled:
        sendWarnMessage(tr("Network cable unplugged"));
        break;
    default:
        clearWarnMessage();
        break;
    }
}

void NetworkPanelContainer::showEvent(QShowEvent *event) {
    if (m_contentWidget == m_dialog->panel()) {
        m_contentWidget->setVisible(true);
    }
}

void NetworkPanelContainer::hideEvent(QHideEvent *event) {
    if (m_contentWidget == m_dialog->panel()) {
        m_contentWidget->setVisible(false);
    }
}

void NetworkPanelContainer::setContentWidget(QWidget *content)
{
    if (m_contentWidget) {
        m_contentWidget->removeEventFilter(this);
        m_contentWidget->setParent(m_savedParent);
    }
    m_contentWidget = content;
    m_savedParent = content->parentWidget();
    // Note: for m_dialog->panel(), setting parent to container means controlling its life span.
    // As container has the same life span as panel, this is not a problem at least now.
    m_contentWidget->setParent(this);
    resize(content->size());
    content->installEventFilter(this);
    content->show();
}

bool NetworkPanelContainer::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_contentWidget) {
        if (event->type() == QEvent::Resize) {
            QResizeEvent *rsEvent = dynamic_cast<QResizeEvent *>(event);
            resize(rsEvent->size());
        }
    }
    return false;
}
} // namespace module
} // namespace dss
