// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "datetimeplugin.h"
#include "tipswidget.h"
#include "utils.h"
#include "plugins-logging-category.h"
#include "regionFormat.h"

#include <DDBusSender>
#include <DConfig>

#include <QDebug>
#include <QDBusConnectionInterface>

#include <unistd.h>

Q_LOGGING_CATEGORY(DOCK_DATETIME, "org.deepin.dde.dock.datetime")
Q_DECLARE_METATYPE(QMargins)

#define DATETIME_KEY "datetime"
#define PLUGIN_STATE_KEY "enable"
#define TIME_FORMAT_KEY "Use24HourFormat"
using namespace Dock;
DatetimePlugin::DatetimePlugin(QObject *parent)
    : QObject(parent)
    , m_centralWidget(nullptr)
    , m_dateTipsLabel(nullptr)
    , m_calendarPopup(nullptr)
    , m_refershTimer(nullptr)
    , m_interface(nullptr)
    , m_pluginLoaded(false)
    , m_RegionFormatModel(nullptr)
{

}

DatetimePlugin::~DatetimePlugin()
{

}

PluginsItemInterface::PluginSizePolicy DatetimePlugin::pluginSizePolicy() const
{
    return PluginsItemInterface::Custom;
}

const QString DatetimePlugin::pluginName() const
{
    return "datetime";
}

const QString DatetimePlugin::pluginDisplayName() const
{
    return tr("Datetime");
}

void DatetimePlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    // transfer config
    QSettings settings("deepin", "dde-dock-datetime");
    if (QFile::exists(settings.fileName())) {
        Dock::DisplayMode mode = displayMode();
        const QString key = QString("pos_%1_%2").arg(pluginName()).arg(mode);
        proxyInter->saveValue(this, key, settings.value(key, mode == Dock::DisplayMode::Fashion ? 6 : -1));
        QFile::remove(settings.fileName());
    }

    if (pluginIsDisable()) {
        return;
    }

    loadPlugin();
}

void DatetimePlugin::loadPlugin()
{
    if (m_pluginLoaded)
        return;

    m_pluginLoaded = true;

    if (!m_RegionFormatModel) {
        m_RegionFormatModel = new RegionFormat(this);
    }

    m_dateTipsLabel.reset(new TipsWidget);
    m_calendarPopup.reset(new SidebarCalendarWidget(m_RegionFormatModel));
    m_refershTimer = new QTimer(this);
    m_dateTipsLabel->setObjectName("datetime");

    m_refershTimer->setInterval(1000);
    m_refershTimer->start();

    m_centralWidget.reset(new DatetimeWidget(m_RegionFormatModel));

    connect(m_centralWidget.data(), &DatetimeWidget::requestUpdateGeometry, this, [this] {
        m_centralWidget.data()->setFixedSize(m_centralWidget.data()->sizeHint());
        m_proxyInter->itemUpdate(this, pluginName());
    });
    connect(m_refershTimer, &QTimer::timeout, this, &DatetimePlugin::updateCurrentTimeString);
    connect(m_calendarPopup.data(), &SidebarCalendarWidget::jumpButtonClicked, this, [this]() {
        m_proxyInter->requestSetAppletVisible(this, DATETIME_KEY, false);
    });

    m_proxyInter->itemAdded(this, pluginName());

    pluginSettingsChanged();
}

void DatetimePlugin::pluginStateSwitched()
{
    m_proxyInter->saveValue(this, PLUGIN_STATE_KEY, pluginIsDisable());

    refreshPluginItemsVisible();
}

bool DatetimePlugin::pluginIsDisable()
{
    bool defaultValue = true;
    auto dconfig = Dtk::Core::DConfig::create("org.deepin.dde.tray-loader", "org.deepin.dde.dock.plugin.common", "");
    if (dconfig) {
        defaultValue = dconfig->value("defaultDockedPlugins", QStringList()).toStringList().contains(pluginName());
        dconfig->deleteLater();
    }
    return !(m_proxyInter->getValue(this, PLUGIN_STATE_KEY, defaultValue).toBool());
}

int DatetimePlugin::itemSortKey(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 6).toInt();
}

void DatetimePlugin::setSortKey(const QString &itemKey, const int order)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

QWidget *DatetimePlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_centralWidget.data();
}

QWidget *DatetimePlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    return m_dateTipsLabel.data();
}

QWidget *DatetimePlugin::itemPopupApplet(const QString &itemKey)
{
    if (itemKey == DATETIME_KEY) {
        return m_calendarPopup.data();
    }

    return nullptr;
}

const QString DatetimePlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;

    QMap<QString, QVariant> settings;
    settings["itemId"] = "settings";
    if (m_centralWidget->is24HourFormat())
        settings["itemText"] = tr("12-hour time");
    else
        settings["itemText"] = tr("24-hour time");
    settings["isActive"] = true;
    items.push_back(settings);

    if (!QFile::exists(ICBC_CONF_FILE)) {
        QMap<QString, QVariant> open;
        open["itemId"] = "open";
        open["itemText"] = tr("Time settings");
        open["isActive"] = true;
        items.push_back(open);
    }

    QMap<QString, QVariant> calendar;
    calendar["itemId"] = "calendar";
    calendar["itemText"] = tr("Open the calendar");
    calendar["isActive"] = true;
    items.push_back(calendar);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DatetimePlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    if (menuId == "open") {
        DDBusSender()
                .service("com.deepin.dde.ControlCenter")
                .interface("com.deepin.dde.ControlCenter")
                .path("/com/deepin/dde/ControlCenter")
                .method(QString("ShowModule"))
                .arg(QString("datetime"))
                .call();
    } else if (menuId == "settings") {
        const bool is24HourFormat = m_centralWidget->is24HourFormat();
        m_centralWidget->set24HourFormat(!is24HourFormat);
    } else {
        QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.Calendar /com/deepin/Calendar com.deepin.Calendar.RaiseWindow");
    }
}

void DatetimePlugin::pluginSettingsChanged()
{
    if (!m_pluginLoaded)
        return;

    refreshPluginItemsVisible();
}

void DatetimePlugin::updateCurrentTimeString()
{
    const QDateTime currentDateTime = QDateTime::currentDateTime();

    // 实时刷新日期，防止日期显示错误
    m_centralWidget->updateDateTimeString();
    m_dateTipsLabel->setText(m_centralWidget->getDateTime());

    const QString currentString = currentDateTime.toString("yyyy/MM/dd hh:mm");

    if (currentString == m_currentTimeString)
        return;

    m_currentTimeString = currentString;
    //当时间显示格式为12小时制且格式为0:00时，当从9:59变到10:00时，插件宽度需要变化
    Q_EMIT m_centralWidget->requestUpdateGeometry();
}

void DatetimePlugin::refreshPluginItemsVisible()
{
    if (!pluginIsDisable()) {

        if (!m_pluginLoaded) {
            loadPlugin();
            return;
        }
        m_proxyInter->itemAdded(this, pluginName());
    } else {
        m_proxyInter->itemRemoved(this, pluginName());
    }
}

Dock::PluginFlags DatetimePlugin::flags() const
{
    return Dock::Type_Tool | Dock::Attribute_Normal;
}

QString DatetimePlugin::message(const QString &message)
{
    QJsonObject msgObj = Utils::getRootObj(message);
    if (msgObj.isEmpty()) {
        return "{}";
    }

    QJsonObject retObj;
    const QString &cmdType = msgObj.value(Dock::MSG_TYPE).toString();
    if (cmdType == Dock::MSG_DOCK_PANEL_SIZE_CHANGED) {
        const QJsonObject sizeObj = msgObj.value(Dock::MSG_DATA).toObject();
        int width = sizeObj["width"].toInt();
        int height = sizeObj["height"].toInt();
        if (m_centralWidget) {
            m_centralWidget.data()->setDockPanelSize(QSize(width, height));
        }
    } else if (cmdType == Dock::MSG_PLUGIN_PROPERTY) {
        QMap<QString, QVariant> map;
        map[Dock::PLUGIN_PROP_NEED_CHAMELEON] = true;
        map[Dock::PLUGIN_PROP_CHAMELEON_MARGIN] = QVariant::fromValue(QMargins(0, 0, 0, 0));
        retObj[Dock::MSG_DATA] = QJsonValue::fromVariant(map);
    }

    return Utils::toJson(retObj);
}

void DatetimePlugin::positionChanged(const Dock::Position position)
{
    Q_UNUSED(position);
    if (m_centralWidget) {
        // 任务栏位置发生改变的时候需要重新设置窗口大小
        if (m_RegionFormatModel)  {
            m_RegionFormatModel->onDockPositionChanged(position);
        }
        m_centralWidget.data()->setFixedSize(m_centralWidget.data()->sizeHint());
        m_centralWidget.data()->dockPositionChanged();
    }
}
