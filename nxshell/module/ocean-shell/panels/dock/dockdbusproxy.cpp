// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "docksettings.h"
#include "dsglobal.h"
#include "constants.h"
#include "dockpanel.h"
#include "dockdbusproxy.h"

#include <QObject>

#include <DWindowManagerHelper>
#include <appletbridge.h>

DGUI_USE_NAMESPACE
DS_USE_NAMESPACE

namespace dock {
DockDBusProxy::DockDBusProxy(DockPanel* parent)
    : QObject(parent)
    , m_oldDockApplet(nullptr)
    , m_multitaskviewApplet(nullptr)
    , m_trayApplet(nullptr)
{
    registerPluginInfoMetaType();

    connect(DockSettings::instance(), &DockSettings::pluginsVisibleChanged, this, [this] (const QVariantMap &pluginsVisible) {
        setPluginVisible("org.lingmo.ds.dock.multitaskview", pluginsVisible);
    });
    connect(parent, &DockPanel::rootObjectChanged, this, [this]() {
        auto pluginsVisible = DockSettings::instance()->pluginsVisible();
        setPluginVisible("org.lingmo.ds.dock.multitaskview", pluginsVisible);
    });

    // Communicate with the other module
    auto getOtherApplet = [ = ] {
        {
            DAppletBridge bridge("org.lingmo.ds.dock.tray");
            m_trayApplet = bridge.applet();
        }
        {
            DAppletBridge bridge("org.lingmo.ds.dock.multitaskview");
            m_multitaskviewApplet = bridge.applet();
        }

        return m_trayApplet && m_multitaskviewApplet;
    };

    // TODO: DQmlGlobal maybe missing a  signal which named `appletListChanged`?
    QTimer *timer = new QTimer;
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [ = ] {
        if (getOtherApplet()) {
            timer->stop();
            timer->deleteLater();
        }
    });
    timer->start();
}

DockPanel* DockDBusProxy::parent() const
{
    return static_cast<DockPanel*>(QObject::parent());
}

QString DockDBusProxy::getAppID(const QString &desktopfile)
{
    const QString desktopLeft = "/applications/";
    const QString desktopSuffix = ".desktop";
    return desktopfile.mid(desktopfile.lastIndexOf(desktopLeft) + desktopLeft.size(), desktopfile.lastIndexOf(desktopSuffix) - desktopfile.lastIndexOf(desktopLeft) - desktopLeft.size());
}

QRect DockDBusProxy::geometry()
{
    return parent()->window() ? parent()->window()->geometry() : QRect();
}

void DockDBusProxy::setPluginVisible(const QString &pluginId, const QVariantMap &pluginsVisible)
{
    if (DAppletBridge bridge(pluginId); auto item = bridge.applet()) {
        DockItemInfo itemInfo;
        QMetaObject::invokeMethod(item, "dockItemInfo", Qt::DirectConnection, qReturnArg(itemInfo));
        QString itemKey = itemInfo.itemKey;
        if (pluginsVisible.contains(itemKey)) {
            QMetaObject::invokeMethod(item, "setVisible", Qt::QueuedConnection, pluginsVisible[itemKey].toBool());
        } else {
            auto settingPluginsVisible = DockSettings::instance()->pluginsVisible();
            settingPluginsVisible[itemKey] = true;
            DockSettings::instance()->setPluginsVisible(settingPluginsVisible);
        }
    }
}

QRect DockDBusProxy::frontendWindowRect()
{
    return parent()->frontendWindowRect();
}

Position DockDBusProxy::position()
{
    return parent()->position();
}

void DockDBusProxy::setPosition(Position position)
{
    parent()->setPosition(position);
}

HideMode DockDBusProxy::hideMode()
{
    return parent()->hideMode();
}

void DockDBusProxy::setHideMode(HideMode mode)
{
    parent()->setHideMode(mode);
}

HideState DockDBusProxy::hideState()
{
    return parent()->hideState();
}

uint DockDBusProxy::windowSizeEfficient()
{
    return parent()->dockSize();
}

void DockDBusProxy::setWindowSizeEfficient(uint size)
{
    qDebug() << size;
    parent()->setDockSize(size);
}

uint DockDBusProxy::windowSizeFashion()
{
    return parent()->dockSize();
}

void DockDBusProxy::setWindowSizeFashion(uint size)
{
    parent()->setDockSize(size);
}

int DockDBusProxy::displayMode()
{
    return parent()->itemAlignment();
}

void DockDBusProxy::setDisplayMode(int displayMode)
{
    parent()->setItemAlignment(static_cast<ItemAlignment>(displayMode));
}

bool DockDBusProxy::RequestDock(const QString &desktopFile, int index) {
    Q_UNUSED(index);
    QString id = getAppID(desktopFile);
    if (id.isEmpty())
        return false;

    DAppletBridge bridge("org.lingmo.ds.dock.taskmanager");
    auto appletItem = bridge.applet();
    if (nullptr == appletItem)
        return false;
    bool res = true;
    QMetaObject::invokeMethod(appletItem, "RequestDock", Qt::DirectConnection, Q_RETURN_ARG(bool, res), Q_ARG(QString, id));
    return res;
}

bool DockDBusProxy::IsDocked(const QString &desktopFile)
{
    QString id = getAppID(desktopFile);
    if (id.isEmpty())
        return false;

    DAppletBridge bridge("org.lingmo.ds.dock.taskmanager");
    auto appletItem = bridge.applet();
    if (nullptr == appletItem)
        return false;
    bool res = true;
    QMetaObject::invokeMethod(appletItem, "IsDocked", Qt::DirectConnection, Q_RETURN_ARG(bool, res), Q_ARG(QString, id));
    return res;
}

bool DockDBusProxy::RequestUndock(const QString &desktopFile)
{
    QString id = getAppID(desktopFile);
    DAppletBridge bridge("org.lingmo.ds.dock.taskmanager");
    auto appletItem = bridge.applet();
    if (nullptr == appletItem)
        return false;
    bool res = true;
    QMetaObject::invokeMethod(appletItem, "RequestUndock", Qt::DirectConnection, Q_RETURN_ARG(bool, res), Q_ARG(QString, id));
    return res;
}

QStringList DockDBusProxy::GetLoadedPlugins()
{
    // TODO: implement this function
    return QStringList();
}

DockItemInfos DockDBusProxy::plugins()
{
    DockItemInfos iteminfos;
    if (m_trayApplet) {
        QMetaObject::invokeMethod(m_trayApplet, "dockItemInfos", Qt::DirectConnection, qReturnArg(iteminfos));
    }

    if (m_multitaskviewApplet && DWindowManagerHelper::instance()->hasComposite()) {
        DockItemInfo info;
        if (QMetaObject::invokeMethod(m_multitaskviewApplet, "dockItemInfo", Qt::DirectConnection, qReturnArg(info))) {
            iteminfos.append(info);
        }
    }
    return iteminfos;
}

void DockDBusProxy::ReloadPlugins()
{
    parent()->ReloadPlugins();
}

void DockDBusProxy::callShow()
{
    parent()->callShow();
}

void DockDBusProxy::setItemOnDock(const QString &settingKey, const QString &itemKey, bool visible)
{
    if (itemKey == "multitasking-view" && m_multitaskviewApplet) {
        QMetaObject::invokeMethod(m_multitaskviewApplet, "setVisible", Qt::QueuedConnection, visible);
        auto pluginsVisible = DockSettings::instance()->pluginsVisible();
        pluginsVisible[itemKey] = visible;
        DockSettings::instance()->setPluginsVisible(pluginsVisible);
        Q_EMIT pluginVisibleChanged(itemKey, visible);
    } else if (m_trayApplet) {
        Q_EMIT pluginVisibleChanged(itemKey, visible);
        QMetaObject::invokeMethod(m_trayApplet, "setItemOnDock", Qt::QueuedConnection, settingKey, itemKey, visible);
    }
}

void DockDBusProxy::setPluginVisible(const QString &pluginName, bool visible)
{
    // TODO: implement this function
    Q_UNUSED(pluginName)
    Q_UNUSED(visible)
}

bool DockDBusProxy::getPluginVisible(const QString &pluginName)
{
    // TODO: implement this function
    Q_UNUSED(pluginName)
    return true;
}

QString DockDBusProxy::getPluginKey(const QString &pluginName)
{
    // TODO: implement this function
    Q_UNUSED(pluginName)
    return QString();
}

void DockDBusProxy::resizeDock(int offset, bool dragging)
{
    Q_UNUSED(dragging)
    parent()->setDockSize(offset);
}

bool DockDBusProxy::showInPrimary() const
{
    return parent()->showInPrimary();
}

void DockDBusProxy::setShowInPrimary(bool newShowInPrimary)
{
    if (parent()->showInPrimary() == newShowInPrimary)
        return;

    parent()->setShowInPrimary(newShowInPrimary);
}

}

