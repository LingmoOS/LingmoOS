// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "airplanemodeplugin.h"
#include "airplanemodeitem.h"
#include "utils.h"
#include "plugins-logging-category.h"

#include <DConfig>

#define AIRPLANE_MODE_KEY "airplane-mode-key"
DCORE_USE_NAMESPACE

Q_LOGGING_CATEGORY(AIRPLANE, "org.deepin.dde.dock.airplane-mode")

AirplaneModePlugin::AirplaneModePlugin(QObject *parent)
    : QObject(parent)
    , m_item(new AirplaneModeItem)
    , m_dconfig(DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.network", QString(), this))
    , m_quickPanelWidget(new QuickPanelWidget)
    , m_messageCallback(nullptr)
{
    connect(m_item, &AirplaneModeItem::airplaneEnableChanged, this, &AirplaneModePlugin::onAirplaneEnableChanged);
    connect(m_dconfig, &DConfig::valueChanged, this, &AirplaneModePlugin::updatePluginVisible);
}

AirplaneModePlugin::~AirplaneModePlugin()
{
    if (m_item) {
        delete m_item;
        m_item = nullptr;
    }

    if (m_networkInter) {
        delete m_networkInter;
        m_networkInter = nullptr;
    }

    if (m_bluetoothInter) {
        delete m_bluetoothInter;
        m_bluetoothInter = nullptr;
    }

    if (m_dconfig) {
        delete m_dconfig;
        m_dconfig = nullptr;
    }

    if (m_quickPanelWidget) {
        delete m_quickPanelWidget;
        m_quickPanelWidget = nullptr;
    }
}

const QString AirplaneModePlugin::pluginName() const
{
    return "airplane-mode";
}

const QString AirplaneModePlugin::pluginDisplayName() const
{
    return tr("Airplane Mode");
}

void AirplaneModePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (getAirplaneDconfig()) {
        m_networkInter = new NetworkInter("com.deepin.daemon.Network", "/com/deepin/daemon/Network", QDBusConnection::sessionBus(), this);
        connect(m_networkInter, &NetworkInter::WirelessAccessPointsChanged, this, &AirplaneModePlugin::updatePluginVisible);

        m_bluetoothInter = new BluetoothInter("com.deepin.daemon.Bluetooth", "/com/deepin/daemon/Bluetooth", QDBusConnection::sessionBus(), this);
        connect(m_bluetoothInter, &BluetoothInter::AdapterAdded, this, &AirplaneModePlugin::updatePluginVisible);
        connect(m_bluetoothInter, &BluetoothInter::AdapterRemoved, this, &AirplaneModePlugin::updatePluginVisible);

        QDBusConnection::systemBus().connect("com.deepin.daemon.AirplaneMode",
                                             "/com/deepin/daemon/AirplaneMode",
                                             "org.freedesktop.DBus.Properties",
                                             "PropertiesChanged",
                                             this,
                                             SLOT(onAirplaneModePropertiesChanged(QString, QVariantMap, QStringList)));
    }

    if (supportAirplaneMode()) {
        m_proxyInter->itemAdded(this, AIRPLANE_MODE_KEY);
    }

    refreshAirplaneEnableState();

    m_quickPanelWidget->setDescription(pluginDisplayName());
    m_quickPanelWidget->setIcon(QIcon::fromTheme("airplanemode-on"));

    connect(m_item, &AirplaneModeItem::requestHideApplet, this, [this] {
        m_proxyInter->requestSetAppletVisible(this, AIRPLANE_MODE_KEY, false);
    });
}

QWidget *AirplaneModePlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == AIRPLANE_MODE_KEY) {
        return m_item->iconWidget();
    }

    if (itemKey == Dock::QUICK_ITEM_KEY) {
        return m_quickPanelWidget;
    }

    return nullptr;
}

QWidget *AirplaneModePlugin::itemTipsWidget(const QString &itemKey)
{
    if (itemKey == AIRPLANE_MODE_KEY) {
        return m_item->tipsWidget();
    }

    return nullptr;
}

QWidget *AirplaneModePlugin::itemPopupApplet(const QString &itemKey)
{
    if (!supportAirplaneMode()) {
        return nullptr;
    }

    if (itemKey == AIRPLANE_MODE_KEY) {
        return m_item->popupApplet();
    }

    return nullptr;
}

const QString AirplaneModePlugin::itemContextMenu(const QString &itemKey)
{
    if (!supportAirplaneMode()) {
        return QString();
    }

    if (itemKey == AIRPLANE_MODE_KEY) {
        return m_item->contextMenu();
    }

    return QString();
}

void AirplaneModePlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    if (itemKey == AIRPLANE_MODE_KEY) {
        m_item->invokeMenuItem(menuId, checked);
    }
}

int AirplaneModePlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    return m_proxyInter->getValue(this, key, -1).toInt();
}

void AirplaneModePlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    m_proxyInter->saveValue(this, key, order);
}

void AirplaneModePlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == AIRPLANE_MODE_KEY) {
        m_item->refreshIcon();
    }
}

void AirplaneModePlugin::pluginSettingsChanged()
{
    updatePluginVisible();
}

void AirplaneModePlugin::refreshAirplaneEnableState()
{
    onAirplaneEnableChanged(m_item->airplaneEnable());
}

void AirplaneModePlugin::onAirplaneEnableChanged(bool enable)
{
    if (!m_proxyInter)
        return;

    if (supportAirplaneMode()) {
        m_proxyInter->itemAdded(this, AIRPLANE_MODE_KEY);
    }
}

void AirplaneModePlugin::updatePluginVisible()
{
    const bool support = supportAirplaneMode();
    notifySupportFlagChanged(support);
    if (!support) {
        m_proxyInter->itemRemoved(this, AIRPLANE_MODE_KEY);
    } else {
        m_proxyInter->itemAdded(this, AIRPLANE_MODE_KEY);
    }
}

bool AirplaneModePlugin::supportAirplaneMode() const
{
    // dde-dconfig配置优先级高于设备优先级
    bool bAirplane = false;
    if (m_dconfig && m_dconfig->isValid()) {
        bAirplane = m_dconfig->value("networkAirplaneMode", false).toBool();
    }
    if (!bAirplane) {
        return bAirplane;
    }

    // 蓝牙和无线网络,只要有其中一个就允许显示飞行模式
    QDBusInterface inter("com.deepin.system.Bluetooth",
                    "/com/deepin/system/Bluetooth",
                    "com.deepin.system.Bluetooth",
                    QDBusConnection::systemBus());
    if (inter.isValid()) {
        QDBusReply<QString> reply = inter.call("GetAdapters");
        QString replyStr = reply.value();
        QJsonDocument json = QJsonDocument::fromJson(replyStr.toUtf8());
        QJsonArray array = json.array();
        if (array.size() > 0 && !array[0].toObject()["Path"].toString().isEmpty()) {
            return true;
        }
    }

    QDBusInterface networkInter("org.freedesktop.NetworkManager",
                                "/org/freedesktop/NetworkManager",
                                "org.freedesktop.NetworkManager",
                                QDBusConnection::systemBus());
    if (networkInter.isValid()) {
        QDBusReply<QList<QDBusObjectPath>> reply = networkInter.call("GetAllDevices");
        const QList<QDBusObjectPath> &replyStrList = reply.value();
        for (const QDBusObjectPath &objPath : replyStrList) {
            QDBusInterface deviceInter("org.freedesktop.NetworkManager",
                                        objPath.path(),
                                        "org.freedesktop.NetworkManager.Device",
                                        QDBusConnection::systemBus());
            if (deviceInter.isValid()) {
                QVariant reply = deviceInter.property("DeviceType");
                // 2 NM_DEVICE_TYPE_WIFI
                if (2 == reply.toUInt()) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool AirplaneModePlugin::getAirplaneDconfig() const
{
    bool airplane = false;
    if (m_dconfig && m_dconfig->isValid()) {
        airplane = m_dconfig->value("networkAirplaneMode", false).toBool();
    }
    return airplane;
}

void AirplaneModePlugin::onAirplaneModePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    if (changedProperties.contains("HasAirplaneMode")) {
        updatePluginVisible();
    }

    // 插入蓝牙模块的时候，com.deepin.daemon.AirplaneMode的`Enabled`属性快速变化，libdframeworkdbus提供的enabledChanged信号有遗漏，导致状态错误
    if (changedProperties.contains("Enabled")) {
        updatePluginVisible();
        m_quickPanelWidget->setWidgetState(changedProperties.value("Enabled").toBool() ? SignalQuickPanel::WS_ACTIVE : SignalQuickPanel::WS_NORMAL);
    }
}

QString AirplaneModePlugin::message(const QString &message)
{
    QJsonObject msgObj = Utils::getRootObj(message);
    if (msgObj.isEmpty()) {
        return "{}";
    }

    QJsonObject retObj;
    QString cmdType = msgObj.value(Dock::MSG_TYPE).toString();
    if (cmdType == Dock::MSG_GET_SUPPORT_FLAG) {
        retObj[Dock::MSG_SUPPORT_FLAG] = supportAirplaneMode();
    }

    return Utils::toJson(retObj);
}

void AirplaneModePlugin::notifySupportFlagChanged(bool supportFlag)
{
    if (m_messageCallback) {
        return;
    }

    QJsonObject msg;
    msg[Dock::MSG_TYPE] = Dock::MSG_SUPPORT_FLAG_CHANGED;
    msg[Dock::MSG_DATA] = supportFlag;
    m_messageCallback(this, Utils::toJson(msg));
}
