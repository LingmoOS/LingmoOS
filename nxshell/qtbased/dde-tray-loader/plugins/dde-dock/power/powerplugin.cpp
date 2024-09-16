// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "powerplugin.h"

#include "utils.h"
#include "tipswidget.h"
#include "dbus/dbusaccount.h"
#include "plugins-logging-category.h"

#include <DDBusSender>
#include <DConfig>

#include <DConfig>
#include <QIcon>

#define PLUGIN_STATE_KEY "enable"

Q_LOGGING_CATEGORY(DOCK_POWER, "org.deepin.dde.dock.power")

using namespace Dock;

PowerPlugin::PowerPlugin(QObject *parent)
    : QObject(parent)
    , m_pluginLoaded(false)
    , m_showTimeToFull(true)
    , m_powerStatusWidget(nullptr)
    , m_tipsLabel(new TipsWidget)
    , m_systemPowerInter(nullptr)
    , m_powerInter(new DBusPower(this))
    , m_batteryStateChangedTimer(new QTimer(this))
    , m_messageCallback(nullptr)
    , m_config(Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.dock.plugin.power", "", this))
{
    m_tipsLabel->setVisible(false);
    m_tipsLabel->setObjectName("battery");
    m_batteryStateChangedTimer->setSingleShot(true);
    connect(m_batteryStateChangedTimer, &QTimer::timeout, this, &PowerPlugin::refreshTipsData);
}

const QString PowerPlugin::pluginName() const
{
    return "battery";
}

const QString PowerPlugin::pluginDisplayName() const
{
    return tr("Battery");
}

QWidget *PowerPlugin::itemWidget(const QString &itemKey)
{
    if (itemKey == POWER_KEY)
        return m_powerStatusWidget->itemWidget();

    return nullptr;
}

QWidget *PowerPlugin::itemPopupApplet(const QString &itemKey)
{
    if (itemKey == POWER_KEY) {
        return m_powerStatusWidget->popupApplet();
    }

    return nullptr;
}

QWidget *PowerPlugin::itemTipsWidget(const QString &itemKey)
{
    const BatteryPercentageMap data = m_powerInter->batteryPercentage();

    if (data.isEmpty()) {
        return nullptr;
    }

    m_tipsLabel->setObjectName(itemKey);

    refreshTipsData();

    return m_tipsLabel.data();
}

void PowerPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (!pluginIsDisable()) {
        loadPlugin();
    }
}

void PowerPlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, PLUGIN_STATE_KEY, pluginIsDisable());

    refreshPluginItemsVisible();
}

bool PowerPlugin::pluginIsDisable()
{
    bool defaultValue = true;
    auto dconfig = Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.dock.plugin.common", "");
    if (dconfig) {
        defaultValue = dconfig->value("defaultDockedPlugins", QStringList()).toStringList().contains(pluginName());
        dconfig->deleteLater();
    }
    return !m_proxyInter->getValue(this, PLUGIN_STATE_KEY, defaultValue).toBool();
}

const QString PowerPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != POWER_KEY) {
        return QString();
    }

    QList<QVariant> items;
    items.reserve(6);

    if (!QFile::exists(ICBC_CONF_FILE)) {
        QMap<QString, QVariant> power;
        power["itemId"] = "power";
        power["itemText"] = tr("Power settings");
        power["isActive"] = true;
        items.push_back(power);
    }

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void PowerPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == "power") {
        DDBusSender()
                .service("com.deepin.dde.ControlCenter")
                .interface("com.deepin.dde.ControlCenter")
                .path("/com/deepin/dde/ControlCenter")
                .method(QString("ShowModule"))
                .arg(QString("power"))
                .call();
    }
}

void PowerPlugin::refreshIcon(const QString &itemKey)
{
    if (itemKey == POWER_KEY) {
        m_powerStatusWidget->refreshIcon();
    }
}

int PowerPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    return m_proxyInter->getValue(this, key, 4).toInt();
}

void PowerPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);

    m_proxyInter->saveValue(this, key, order);
}

void PowerPlugin::pluginSettingsChanged()
{
    refreshPluginItemsVisible();
}

void PowerPlugin::updateBatteryVisible()
{
    const bool exist = !m_powerInter->batteryPercentage().isEmpty();
    notifySupportFlagChanged(exist);
    if (!exist)
        m_proxyInter->itemRemoved(this, POWER_KEY);
    else if (exist && !pluginIsDisable())
        m_proxyInter->itemAdded(this, POWER_KEY);
}

void PowerPlugin::loadPlugin()
{
    if (m_pluginLoaded) {
        qCDebug(DOCK_POWER) << "Load power plugin, power plugin has been loaded";
        return;
    }

    m_pluginLoaded = true;

    m_powerStatusWidget.reset(new PowerStatusWidget);
    connect(m_powerStatusWidget.data(), &PowerStatusWidget::requestHideApplet, this, [this] {
        m_proxyInter->requestSetAppletVisible(this, POWER_KEY, false);
    });

    m_systemPowerInter = new SystemPowerInter("com.deepin.system.Power", "/com/deepin/system/Power", QDBusConnection::systemBus(), this);
    m_systemPowerInter->setSync(true);

    connect(m_config, &Dtk::Core::DConfig::valueChanged, this, &PowerPlugin::onDConfigValueChanged);
    connect(m_systemPowerInter, &SystemPowerInter::BatteryStatusChanged, this, [&](uint value) {
        // 从状态改变，到计算出相关电量数据，需要时间，避免期间数据有较大误差
        if (value == BatteryState::CHARGING) {
            // 插入电源后，20秒内算作预充电时间，此时计算剩余充电时间是不准确的
            m_batteryStateChangedTimer->start(20 * 1000);
        } else if (value == BatteryState::DIS_CHARGING) {
            // 插入电源后，20秒内延迟显示剩余时间
            m_batteryStateChangedTimer->start(20 * 1000);
        } else if (value == BatteryState::NOT_CHARGED || value == BatteryState::FULLY_CHARGED) {
            // 电池一些固定状态，无需再计算
            m_batteryStateChangedTimer->stop();
        }
        refreshTipsData();
    });
    connect(m_systemPowerInter, &SystemPowerInter::BatteryTimeToEmptyChanged, this, &PowerPlugin::refreshTipsData);
    connect(m_systemPowerInter, &SystemPowerInter::BatteryTimeToFullChanged, this, &PowerPlugin::refreshTipsData);
    connect(m_systemPowerInter, &SystemPowerInter::BatteryPercentageChanged, this, &PowerPlugin::refreshTipsData);

    connect(m_powerInter, &DBusPower::BatteryPercentageChanged, this, &PowerPlugin::updateBatteryVisible);

    updateBatteryVisible();

    onDConfigValueChanged("showTimeToFull");
    m_powerStatusWidget->refreshIcon();
}

void PowerPlugin::refreshPluginItemsVisible()
{
    if (pluginIsDisable()) {
        m_proxyInter->itemRemoved(this, POWER_KEY);
    } else {
        if (!m_pluginLoaded) {
            loadPlugin();
            return;
        }
        updateBatteryVisible();
    }
}

void PowerPlugin::onDConfigValueChanged(const QString &key)
{
    if (key != "showTimeToFull") {
        return;
    }

    if (m_config->isValid()) {
        m_showTimeToFull = m_config->value("showTimeToFull").toBool();
    }

    refreshTipsData();
}

void PowerPlugin::batteryTimeToDisplayData(const qulonglong &time, uint &hour, uint &minute)
{
    const QDateTime &timeTmp = QDateTime::fromTime_t(time).toUTC();
    hour = timeTmp.toString("hh").toUInt();
    minute = timeTmp.toString("mm").toUInt();
    uint sencond = timeTmp.toString("ss").toInt();
    if (sencond > 0) {
        // 因为界面不显示秒数。比如0小时0分钟33秒时，应该进位显示0小时1分钟
        minute += 1;
    }
    if (minute == 60) {
        // 当分钟为59时，秒数再进位，会显示60分钟，需要进位到小时
        hour += 1;
        minute = 0;
    }
}

void PowerPlugin::refreshTipsData()
{
    const uint percentage = qMin(100.0, qMax(0.0, m_systemPowerInter->batteryPercentage()));
    const QString value = QString("%1%").arg(std::round(percentage));
    const int batteryState = m_systemPowerInter->batteryStatus();

    if (m_batteryStateChangedTimer->isActive()) {
        if ((batteryState == BatteryState::CHARGING && m_showTimeToFull)
        || (batteryState == BatteryState::DIS_CHARGING)) {
            // 计算期间
            QString tips = tr("Capacity %1 ...").arg(value);
            m_tipsLabel->setText(tips);
            if (batteryState == BatteryState::CHARGING && m_showTimeToFull) {
                m_powerStatusWidget->refreshBatteryPercentage(value, tr("Charging, calculating estimated charging time"));
            } else {
                m_powerStatusWidget->refreshBatteryPercentage(value, tr("Calculating remaining usage time"));
            }
            return;
        }
    }

    QString tips;
    QString appletTips;
    if (batteryState == BatteryState::DIS_CHARGING || batteryState == BatteryState::UNKNOWN) {
        qulonglong timeToEmpty = m_systemPowerInter->batteryTimeToEmpty();
        uint hour = 0;
        uint min = 0;
        batteryTimeToDisplayData(timeToEmpty, hour, min);
        if (!m_showTimeToFull || timeToEmpty == 0 || (hour == 0 && min == 0)) {
            tips = tr("Capacity %1").arg(value);
            appletTips = "";
        } else {
            if (hour == 0 && min != 0) {
                tips = tr("Capacity %1, %2 min remaining").arg(value).arg(min);
                appletTips = tr("%1 min remaining").arg(min);
            } else if (hour != 0 && min == 0) {
                tips = tr("Capacity %1, %2 hr remaining").arg(value).arg(hour);
                appletTips = tr("%1 hr remaining").arg(hour);
            } else {
                tips = tr("Capacity %1, %2 hr %3 min remaining").arg(value).arg(hour).arg(min);
                appletTips = tr("%1 hr %2 min remaining").arg(hour).arg(min);
            }
        }
    } else if (batteryState == BatteryState::NOT_CHARGED) {
        tips = tr("Capacity %1, not charging").arg(value);
        appletTips = tr("Not charging");
    } else if (batteryState == BatteryState::FULLY_CHARGED || percentage == 100) {
        tips = tr("Capacity %1, fully charged").arg(value);
        appletTips = tr("Fully charged");
    } else {
        qulonglong timeToFull = m_systemPowerInter->batteryTimeToFull();
        uint hour = 0;
        uint min = 0;
        batteryTimeToDisplayData(timeToFull, hour, min);
        if (!m_showTimeToFull) {
            tips = tr("Charging %1").arg(value);
            appletTips = "";
        } else {
            // 充电时间timeToFull可能不为0是一个很小的数值,转换后取的hour和min为0
            if (timeToFull == 0 || (hour == 0 && min == 0)) { // 电量已充満或电量计算中,剩余充满时间会返回0
                tips = tr("Capacity %1 ...").arg(value);
                appletTips = tr("Charging, calculating estimated charging time");
            } else {
                if (hour == 0 && min != 0) {
                    tips = tr("Charging %1, %2 min until full").arg(value).arg(min);
                    appletTips = tr("Charging, %1 min until full").arg(min);
                } else if (hour != 0 && min == 0) {
                    tips = tr("Charging %1, %2 hr until full").arg(value).arg(hour);
                    appletTips = tr("Charging, %1 hr until full").arg(hour);
                } else {
                    tips = tr("Charging %1, %2 hr %3 min until full").arg(value).arg(hour).arg(min),
                    appletTips = tr("Charging, %1 hr %2 min until full").arg(hour).arg(min);
                }
            }
        }
    }

    m_tipsLabel->setText(tips);
    m_powerStatusWidget->refreshBatteryPercentage(value, appletTips);
}

QString PowerPlugin::message(const QString &message)
{
    QJsonObject msgObj = Utils::getRootObj(message);
    if (msgObj.isEmpty()) {
        return "{}";
    }

    QJsonObject retObj;
    QString cmdType = msgObj.value(Dock::MSG_TYPE).toString();
    if (cmdType == Dock::MSG_GET_SUPPORT_FLAG) {
        retObj[Dock::MSG_SUPPORT_FLAG] = !m_powerInter->batteryPercentage().isEmpty();;
    }

    return Utils::toJson(retObj);
}

void PowerPlugin::notifySupportFlagChanged(bool supportFlag)
{
    if (!m_messageCallback) {
        return;
    }

    QJsonObject msg;
    msg[Dock::MSG_TYPE] = Dock::MSG_SUPPORT_FLAG_CHANGED;
    msg[Dock::MSG_DATA] = supportFlag;
    m_messageCallback(this, Utils::toJson(msg));
}
