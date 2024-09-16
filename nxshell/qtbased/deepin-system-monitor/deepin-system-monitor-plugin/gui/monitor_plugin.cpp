// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitor_plugin.h"
#include "helper.hpp"
#include "ddlog.h"
// Qt
#include <QDBusConnectionInterface>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DDBusSender>
#include <QIcon>
#include <DApplication>
#include <QGSettings>
#include <QPainter>
#include <QFile>

namespace constantVal {
const QString PLUGIN_STATE_KEY = "enable";
}

DWIDGET_USE_NAMESPACE
using namespace DDLog;
MonitorPlugin::MonitorPlugin(QObject *parent)
    : QObject(parent), m_pluginLoaded(false), m_dataTipsLabel(nullptr), m_refershTimer(new QTimer(this))
#ifdef USE_API_QUICKPANEL20
      ,
      m_quickPanelWidget(new QuickPanelWidget),
      m_messageCallback(nullptr)
#endif
{
    connect(m_refershTimer, &QTimer::timeout, this, &MonitorPlugin::udpateTipsInfo);
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-]";
}

const QString MonitorPlugin::pluginName() const
{
    // 不要修改这个名字
    return QString("system-monitor");
}

const QString MonitorPlugin::pluginDisplayName() const
{
    return DApplication::translate("Plugin.DisplayName", "System Monitor");
}

void MonitorPlugin::init(PluginProxyInterface *proxyInter)
{
    // 第一次是为了正确加载翻译，第二次是为了避免影响dock的accessible
    QString applicationName = qApp->applicationName();
    qApp->setApplicationName("deepin-system-monitor-plugin");
    qApp->loadTranslator();
    qApp->setApplicationName(applicationName);

    m_proxyInter = proxyInter;

    if (!pluginIsDisable()) {
        loadPlugin();
    }

#ifdef USE_API_QUICKPANEL20
    m_proxyInter->itemAdded(this, pluginName());
    m_quickPanelWidget->setDescription(pluginDisplayName());
    QString plugIcon = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? "status-system-monitor-dark" : "status-system-monitor";
    QIcon fallbackIcon = QIcon::fromTheme(DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType ? "dsm_pluginicon_dark" : "dsm_pluginicon_light");
    m_quickPanelWidget->setIcon(QIcon::fromTheme(plugIcon, fallbackIcon));
    connect(m_quickPanelWidget, &QuickPanelWidget::clicked, this, &MonitorPlugin::onClickQuickPanel);
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-] QUICKPANEL20";
    QDBusConnection::sessionBus().connect("com.deepin.SystemMonitorPluginPopup", "/com/deepin/SystemMonitorPluginPopup", "com.deepin.SystemMonitorPluginPopup", "sysMonPopVisibleChanged", this, SLOT(onSysMonPopVisibleChanged(bool)));
#endif

    calcCpuRate(m_totalCPU, m_availableCPU);
    calcNetRate(m_down, m_upload);
#ifdef DDE_DOCK_NEW_VERSION
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-] V23";
#endif
}

QWidget *MonitorPlugin::itemWidget(const QString &itemKey)
{
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-]" << itemKey;
#ifdef DDE_DOCK_NEW_VERSION
//    if (itemKey == "system-monitor")
//        return m_itemWidget;
#else
#    ifdef USE_API_QUICKPANEL20
    if (itemKey == Dock::QUICK_ITEM_KEY) {
        return m_quickPanelWidget;
    }
#    endif
    if (itemKey == "system-monitor")
        return m_itemWidget;
#endif
    return nullptr;
}

void MonitorPlugin::pluginStateSwitched()
{
#ifndef DDE_DOCK_NEW_VERSION
    const bool disabledNew = !pluginIsDisable();
    m_proxyInter->saveValue(this, "disabled", disabledNew);
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        if (!m_pluginLoaded) {
            loadPlugin();
            return;
        }
        m_proxyInter->itemAdded(this, pluginName());
    }
#endif
}

#ifndef DDE_DOCK_NEW_VERSION
bool MonitorPlugin::pluginIsDisable()
{
#    ifdef USE_API_QUICKPANEL20
    return false;
#    else
    return m_proxyInter->getValue(this, "disabled", true).toBool();
#    endif
}
#endif

QWidget *MonitorPlugin::itemTipsWidget(const QString &itemKey)
{
    m_dataTipsLabel->setObjectName(itemKey);
    m_dataTipsLabel->setSystemMonitorTipsText(QStringList() << m_cpuStr << m_memStr << m_downloadStr << m_uploadStr);
    return m_dataTipsLabel.data();
}

const QString MonitorPlugin::itemCommand(const QString &itemKey)
{
    if (itemKey == "system-monitor") {
        // Task 30767: 暂时调整为打开系统监视器，隐藏系统监视器插件
        // DBusInterface::getInstance()->showOrHideDeepinSystemMonitorPluginPopupWidget();
        openSystemMonitor();
    }
    return "";

    //    return "dbus-send --print-reply --dest=com.deepin.SystemMonitorPluginPopup /com/deepin/SystemMonitorPluginPopup com.deepin.SystemMonitorPluginPopup.showOrHideDeepinSystemMonitorPluginPopupWidget";
}

void MonitorPlugin::displayModeChanged(const Dock::DisplayMode displayMode)
{
    Q_UNUSED(displayMode);

    if (!pluginIsDisable()) {
        m_itemWidget->update();
#ifdef USE_API_QUICKPANEL20
        m_quickPanelWidget->update();
#endif
    }
}

int MonitorPlugin::itemSortKey(const QString &itemKey)
{
    Dock::DisplayMode mode = displayMode();
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(mode);
    return m_proxyInter->getValue(this, key, DOCK_DEFAULT_POS).toInt();
}

void MonitorPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(displayMode());
    m_proxyInter->saveValue(this, key, order);
}

const QString MonitorPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != "system-monitor")
        return QString();

    QList<QVariant> items;
    items.reserve(1);
    //Add open System monitor mode button
    QMap<QString, QVariant> openSMO;
    openSMO["itemId"] = "openSystemMointor";
    openSMO["itemText"] = DApplication::translate("Plugin.OpenSystemMontitor", "Open");
    openSMO["isActive"] = true;
    items.push_back(openSMO);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;
    return QJsonDocument::fromVariant(menu).toJson();
}

void MonitorPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(checked);
    Q_UNUSED(itemKey);
    if (menuId == "openSystemMointor") {
        openSystemMonitor();
    }
}

#ifdef DDE_DOCK_NEW_VERSION
QIcon MonitorPlugin::icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType)
{
    QString iconName = "dsm_pluginicon_light";
    if (themeType == DGuiApplicationHelper::LightType) {
        // 最小尺寸时，不画背景，采用深色图标
        iconName = "dsm_pluginicon_dark";
    }
    QSize size = QSize(16, 16);
    if (dockPart == DockPart::DCCSetting) {
        size = QSize(20, 20);
    } else if (dockPart == DockPart::QuickPanel) {
        size = QSize(24, 24);
    }
    QIcon icon = QIcon::fromTheme(iconName);
    if (!icon.isNull()) {
        const qreal ratio = m_itemWidget->devicePixelRatioF();
        QSize pixmapSize = QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? size : size * ratio;
        QPixmap pixmap = icon.pixmap(pixmapSize);
        pixmap.setDevicePixelRatio(ratio);
        if (dockPart == DockPart::QuickShow) {
            QPixmap curPixmap(size * ratio);
            pixmap.setDevicePixelRatio(ratio);
            curPixmap.fill(Qt::transparent);
            QPainter painter;
            painter.begin(&curPixmap);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            int pixmapMargin = ceil(1 * ratio);
            painter.drawPixmap(QRect(pixmapMargin, pixmapMargin, curPixmap.width() - pixmapMargin, curPixmap.height() - pixmapMargin), pixmap);
            painter.end();
            return QIcon(curPixmap);
        } else {
            return QIcon(pixmap);
        }
    }
    return icon;
}
#endif

void MonitorPlugin::udpateInfo()
{
    // memory
    qlonglong memory = 0;
    qlonglong memoryAll = 0;
    calcMemRate(memory, memoryAll);
    m_memStr = QString("%1").arg(memory * 100.0 / memoryAll, 1, 'f', 1, QLatin1Char(' ')) + QString("%");

    // CPU
    qlonglong totalCPU = 0;
    qlonglong availableCPU = 0;
    calcCpuRate(totalCPU, availableCPU);
    double cpuPercent = 0.0;
    if (totalCPU != m_totalCPU) {
        cpuPercent = ((totalCPU - m_totalCPU) - (availableCPU - m_availableCPU)) * 100.0 / (totalCPU - m_totalCPU);
    }
    m_cpuStr = QString("%1").arg(cpuPercent, 1, 'f', 1, QLatin1Char(' ')) + QString("%");
    m_totalCPU = totalCPU;
    m_availableCPU = availableCPU;

    // net
    qlonglong netUpload = 0;
    qlonglong netDownload = 0;
    double downRate = 0.0;
    double upRate = 0.0;

    RateUnit unit = RateByte;
    calcNetRate(netDownload, netUpload);
    downRate = autoRateUnits((netDownload - m_down) / (m_refershTimer->interval() / 1000), unit);
    QString downUnit = setRateUnitSensitive(unit);
    unit = RateByte;
    upRate = autoRateUnits((netUpload - m_upload) / (m_refershTimer->interval() / 1000), unit);
    QString uploadUnit = setRateUnitSensitive(unit);
    m_downloadStr = QString("%1").arg(downRate, 1, 'f', 1, QLatin1Char(' ')) + downUnit;
    m_uploadStr = QString("%1").arg(upRate, 1, 'f', 1, QLatin1Char(' ')) + uploadUnit;

    m_down = netDownload;
    m_upload = netUpload;
}

void MonitorPlugin::udpateTipsInfo()
{
    udpateInfo();
    m_dataTipsLabel->setSystemMonitorTipsText(QStringList() << m_cpuStr << m_memStr << m_downloadStr << m_uploadStr);
}

void MonitorPlugin::loadPlugin()
{
    if (m_pluginLoaded)
        return;

    m_pluginLoaded = true;

    m_dataTipsLabel.reset(new SystemMonitorTipsWidget);
    m_dataTipsLabel->setObjectName("systemmonitorpluginlabel");

    m_refershTimer->setInterval(1000);
    //    m_refershTimer->start();

    connect(m_dataTipsLabel.get(), &SystemMonitorTipsWidget::visibleChanged, this, [=](bool visible) {
        if (!visible) {
            m_refershTimer->stop();
        } else {
            udpateInfo();
            m_dataTipsLabel->setSystemMonitorTipsText(QStringList() << "..."
                                                                    << "..."
                                                                    << "..."
                                                                    << "...");
            m_refershTimer->start();
        }
    });

    m_itemWidget = new MonitorPluginButtonWidget;

    m_proxyInter->itemAdded(this, pluginName());

    displayModeChanged(displayMode());
}

void MonitorPlugin::calcCpuRate(qlonglong &totalCPU, qlonglong &availableCPU)
{
    totalCPU = availableCPU = 0;
    bool ok = false;
    QFile file("/proc/stat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    QString line = stream.readLine();
    if (!line.isNull()) {
        QStringList list = line.split(QRegExp("\\s{1,}"));
        for (auto v = list.begin() + 1; v != list.end(); ++v)
            totalCPU += (*v).toLongLong(&ok);
        if (list.size() > 4)
            availableCPU = list.at(4).toLongLong(&ok);
    }

    file.close();
}

void MonitorPlugin::calcMemRate(qlonglong &memory, qlonglong &memoryAll)
{
    memory = memoryAll = 0;
    bool ok = false;
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream stream(&file);
    qlonglong buff[16] = { 0 };
    for (int i = 0; i <= 15; ++i) {
        QString line = stream.readLine();
        QStringList list = line.split(QRegExp("\\s{1,}"));
        if (list.size() >= 2) {
            buff[i] = list.at(1).toLongLong(&ok);
        }
    }

    memoryAll = buff[0];
    memory = buff[0] - buff[2];

    file.close();
}

void MonitorPlugin::calcNetRate(qlonglong &netDown, qlonglong &netUpload)
{
    QFile file("/proc/net/dev");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    qlonglong down = 0;
    qlonglong upload = 0;
    QTextStream stream(&file);
    QString line = stream.readLine();
    line = stream.readLine();
    line = stream.readLine();
    while (!line.isNull()) {
        line = line.trimmed();
        QStringList list = line.split(QRegExp("\\s{1,}"));   // match number >= 1 space character

        if (!list.isEmpty()) {
            down = list.at(1).toLongLong();
            upload = list.at(9).toLongLong();
        }

        netDown += down;
        netUpload += upload;
        line = stream.readLine();
    }

    file.close();
}

QString MonitorPlugin::setRateUnitSensitive(MonitorPlugin::RateUnit unit)
{
    switch (unit) {
    case RateUnit::RateBit:
        return QString("BIT/s");
    case RateUnit::RateByte:
        return QString("B/s");
    case RateUnit::RateKb:
        return QString("KB/s");
    case RateUnit::RateMb:
        return QString("MB/s");
    case RateUnit::RateGb:
        return QString("GB/s");
    case RateUnit::RateTb:
        return QString("TB/s");
    default:
        return QString("");
    }
}

double MonitorPlugin::autoRateUnits(qlonglong speed, RateUnit &unit)
{
    /* 自动判断合适的速率单位,默认传进来的是 Byte
     * bit    0 ~ 7 位 (不到 1 字节)
     * Byte   1    ~ 2^10  Byte
     * KB     2^10 ~ 2^20  Byte
     * MB     2^20 ~ 2^30  Byte
     * GB     2^30 ~ 2^40  Byte
     * TB     2^40 ~ 2^50  Byte
     */

    if (unit != RateByte) {
        return -1;
    }

    double sp = 0;
    if (0 <= speed && speed < qPow(2, 10)) {
        unit = RateByte;
        sp = speed;
    } else if (qPow(2, 10) <= speed && speed < qPow(2, 20)) {
        unit = RateKb;
        sp = static_cast<double>(speed / qPow(2, 10) * 1.0);
    } else if (qPow(2, 20) <= speed && speed < qPow(2, 30)) {
        unit = RateMb;
        sp = static_cast<double>(speed / qPow(2, 20) * 1.0);
    } else if (qPow(2, 30) <= speed && speed < qPow(2, 40)) {
        unit = RateGb;
        sp = static_cast<double>(speed / qPow(2, 30) * 1.0);
    } else if (qPow(2, 40) <= speed && speed < qPow(2, 50)) {
        unit = RateTb;
        sp = static_cast<double>(speed / qPow(2, 40) * 1.0);
    } else {
        unit = RateUnknow;
        qCDebug(app) << "本设备网络速率单位传输超过 TB, 或者低于 0 Byte.";
        sp = -1;
    }

    return sp;
}

void MonitorPlugin::openSystemMonitor()
{
    auto launchProcessByAM = [](){
        QDBusMessage message = QDBusMessage::createMethodCall(
                "org.desktopspec.ApplicationManager1",
                "/org/desktopspec/ApplicationManager1/deepin_2dsystem_2dmonitor",
                "org.desktopspec.ApplicationManager1.Application",
                "Launch");

        message << QString("") << QStringList() << QVariantMap();

        QDBusMessage reply = QDBusConnection::sessionBus().call(message);
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qCWarning(app) << "Launch deepin-system-monitor main process error:" << reply.errorMessage();
            return;
        }
    };
    launchProcessByAM();
    //QString cmd("qdbus com.deepin.SystemMonitorMain /com/deepin/SystemMonitorMain com.deepin.SystemMonitorMain.slotRaiseWindow");
    QString cmd("gdbus call -e -d  com.deepin.SystemMonitorMain -o /com/deepin/SystemMonitorMain -m com.deepin.SystemMonitorMain.slotRaiseWindow");
    QTimer::singleShot(200, this, [=]() { QProcess::startDetached(cmd); });
#ifdef USE_API_QUICKPANEL20
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-] right ClickQuickPanel";
    m_proxyInter->requestSetAppletVisible(this, pluginName(), false);
#endif
}

#ifdef USE_API_QUICKPANEL20
MonitorPlugin::~MonitorPlugin()
{
    if (m_quickPanelWidget) {
        delete m_quickPanelWidget;
        m_quickPanelWidget = nullptr;
    }
}

void MonitorPlugin::onClickQuickPanel()
{
    qCInfo(app) << __FUNCTION__ << __LINE__ << "[-MonitorPlugin-] ClickQuickPanel";
    m_proxyInter->requestSetAppletVisible(this, pluginName(), false);

    // Task 30767: 暂时调整为打开系统监视器，隐藏系统监视器插件
    // DBusInterface::getInstance()->showOrHideDeepinSystemMonitorPluginPopupWidget();
    openSystemMonitor();
}

//系统监视器弹窗显示状态改变
void MonitorPlugin::onSysMonPopVisibleChanged(bool visible)
{
    if (!m_messageCallback) {
        qCWarning(app) << "Message callback function is nullptr";
        return;
    }
    QJsonObject msg;
    msg[Dock::MSG_TYPE] = Dock::MSG_ITEM_ACTIVE_STATE;
    msg[Dock::MSG_DATA] = visible;
    QJsonDocument doc;
    doc.setObject(msg);
    m_messageCallback(this, doc.toJson());
}
#endif
