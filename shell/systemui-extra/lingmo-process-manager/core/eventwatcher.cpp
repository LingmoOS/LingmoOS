/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "eventwatcher.h"
#include <kwindowsystem.h>
#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QRegularExpression>
#include <QDebug>
#include <windowmanager/windowmanager.h>

namespace {

const char *free_desktop_service = "org.freedesktop.DBus";
const char *free_desktop_path = "/org/freedesktop/DBus";
const char *free_desktop_interface = "org.freedesktop.DBus";

const char *mpris_service_prefix = "org.mpris.MediaPlayer2.";

const char *status_notifier_watcher_service = "org.kde.StatusNotifierWatcher";
const char *status_notifier_watcher_path = "/StatusNotifierWatcher";
const char *status_notifier_watcher_interface = "org.kde.StatusNotifierWatcher";

int getPidFromStatusNotifierItemServiceName(const QString &serviceName)
{
    QRegularExpression re("\\w+-\\d+-\\d+");
    QRegularExpressionMatch match = re.match(serviceName);
    if (!match.hasMatch()) {
        return 0;
    }

    QStringList nameItems = serviceName.split('-');
    return nameItems.at(1).toInt();
}

}

EventWatcher::EventWatcher(std::unique_ptr<PowerManager> powerManager,
                           std::unique_ptr<DeviceModeManager> deviceModeManager)
    : m_powerManager(std::move(powerManager))
    , m_deviceModeManager(std::move(deviceModeManager))
{
    // we should watch daemon signal all the time for watch resource changed.
    initDaemonInterfaceConnections();
}

void EventWatcher::startWatcher()
{
    initWindowChangedConnections();
    initPowerModeChangedConnections();
    initPowerTypeChangedConnections();
    initDeviceModeChangedConnections();
    initMprisDbusConnections();
    initStatusNotifierWatcherConnections();
    handleExistedWindows();
}

void EventWatcher::stopWatcher()
{
    kdk::WindowManager::self()->disconnect(this);
    m_powerManager.get()->disconnect(this);
    m_deviceModeManager.get()->disconnect(this);
    QDBusConnection::sessionBus().disconnect(
        free_desktop_service,free_desktop_path, free_desktop_interface,
        "NameOwnerChanged", this, SLOT(onMprisDbusNameAcquired(QString,QString,QString)));
}

void EventWatcher::setWindowAddedCallback(WindowChangedCallback callback)
{
    m_windowAddedCallback = std::move(callback);
}

void EventWatcher::setWindowRemovedCallback(WindowChangedCallback callback)
{
    m_windowRemovedCallback = std::move(callback);
}

void EventWatcher::setActiveWindowChangedCallback(WindowChangedCallback callback)
{
    m_activeWindowChangedCallback = std::move(callback);
}

void EventWatcher::setWindowMinimizedCallback(WindowChangedCallback callback)
{
    m_windowMinimizedCallback = std::move(callback);
}

void EventWatcher::setPowerModeChangedCallback(PowerModeChangedCallback callback)
{
    m_powerModeChangedCallback = std::move(callback);
    if (m_powerModeChangedCallback) {
        m_powerModeChangedCallback(m_powerManager->currentPowerMode());
    }
}

void EventWatcher::setPowerTypeChangedCallback(PowerTypeChangedCallback callback)
{
    m_powerTypeChangedCallback = std::move(callback);
}

void EventWatcher::setDeviceModeChangedCallback(DeviceModeChangedCallback callback)
{
    m_deviceModeChangedCallback = std::move(callback);
    if (m_deviceModeChangedCallback) {
        m_deviceModeChangedCallback(m_deviceModeManager->currentDeviceMode());
    }
}

void EventWatcher::setMprisDbusAddedCallback(MprisDbusAddedCallback callback)
{
    m_mprisDbusAddedCallback = std::move(callback);
}

void EventWatcher::setStatusNotifierItemRegisteredCallback(StatusNotifierItemRegisteredCallback callback)
{
    m_statusNotifierItemRegisteredCallback = std::move(callback);
}

void EventWatcher::setResourceWarningCallback(ResourceWarningCallback callback)
{
    m_resourceWarningCallback = std::move(callback);
}

sched_policy::PowerType EventWatcher::powerType() const
{
    return m_powerManager->currentPowerType();
}

sched_policy::DeviceMode EventWatcher::deviceMode() const
{
    return m_deviceModeManager->currentDeviceMode();
}

void EventWatcher::handleSoftFreezeModeEnabledChanged(bool enabled)
{
    m_deviceModeManager->handleSoftFreezeModeEnabledChanged(enabled);
}

void EventWatcher::onStatusNotifierItemRegistered(QString itemName)
{
    int pid = getPidFromStatusNotifierItemServiceName(itemName);
    qDebug() << "Status notifier item registered: " << pid;
    if (m_statusNotifierItemRegisteredCallback) {
        m_statusNotifierItemRegisteredCallback(pid);
    }
}

void EventWatcher::onMprisDbusNameAcquired(QString name, QString newOwner, QString oldOwner)
{
    Q_UNUSED(newOwner)
    Q_UNUSED(oldOwner)

    if (!name.contains(mpris_service_prefix)) {
        return;
    }
    QDBusInterface mediaInterface(
        free_desktop_service, free_desktop_path,
        free_desktop_service, QDBusConnection::sessionBus());

    QDBusPendingReply<quint32> pidReply =
        mediaInterface.call(QStringLiteral("GetConnectionUnixProcessID"), name);
    quint32 pid = pidReply.value();
    if (pid > 0 && m_mprisDbusAddedCallback) {
        m_mprisDbusAddedCallback((int)pid, name.toStdString());
    }
}

void EventWatcher::initWindowChangedConnections()
{
    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded,
                     this, [this](kdk::WindowId wid) {
                        if (m_windowAddedCallback)
                            m_windowAddedCallback(wid.toString().toStdString());
                        });

    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowRemoved,
                     this, [this](kdk::WindowId wid) {
                        if (m_windowRemovedCallback)
                            m_windowRemovedCallback(wid.toString().toStdString());
                        });

    connect(kdk::WindowManager::self(), &kdk::WindowManager::activeWindowChanged,
            this, [this](kdk::WindowId wid) {
                if (m_activeWindowChangedCallback)
                    m_activeWindowChangedCallback(wid.toString().toStdString());
                });

    connect(kdk::WindowManager::self(), &kdk::WindowManager::windowChanged,
                     this, [this](kdk::WindowId wid) {
        auto windowInfo = kdk::WindowManager::getwindowInfo(wid);
        if (windowInfo.isMinimized() && m_windowMinimizedCallback) {
            m_windowMinimizedCallback(wid.toString().toStdString());
        }
    });
}

void EventWatcher::initPowerModeChangedConnections()
{
    connect(m_powerManager.get(), &PowerManager::powerModeChanged,
            this, [this](sched_policy::PowerMode powerMode) {
                if (m_powerModeChangedCallback) {
                    m_powerModeChangedCallback(powerMode);
                }
            });
}

void EventWatcher::initPowerTypeChangedConnections()
{
    connect(m_powerManager.get(), &PowerManager::powerTypeChanged,
            this, [this](sched_policy::PowerType powerType) {
                if (m_powerTypeChangedCallback) {
                    m_powerTypeChangedCallback(powerType);
                }
            });
}

void EventWatcher::initDeviceModeChangedConnections()
{
    connect(m_deviceModeManager.get(), &DeviceModeManager::deviceModeChanged,
            this, [this](sched_policy::DeviceMode deviceMode) {
                if (m_deviceModeChangedCallback) {
                    m_deviceModeChangedCallback(deviceMode);
                }
            });
}

void EventWatcher::initMprisDbusConnections()
{
    QDBusConnection::sessionBus().connect(
        free_desktop_service,free_desktop_path, free_desktop_interface,
        "NameOwnerChanged", this, SLOT(onMprisDbusNameAcquired(QString,QString,QString)));
}

void EventWatcher::initStatusNotifierWatcherConnections()
{
    QDBusConnection::sessionBus().connect(
        status_notifier_watcher_service, status_notifier_watcher_path, status_notifier_watcher_interface,
        "StatusNotifierItemRegistered", this, SLOT(onStatusNotifierItemRegistered(QString)));
}

void EventWatcher::initDaemonInterfaceConnections()
{
    connect(&m_daemonInterface, &DaemonDbusInterface::ResourceThresholdWarning, this,
            [this](const std::string& resource, int level) {
        if (m_resourceWarningCallback) {
            m_resourceWarningCallback(resource, level);
        } else {
            qDebug() << "Resource Warning callback is empty";
        }
    });
}

void EventWatcher::handleExistedWindows()
{
    handleAddedWindows();
    handleActivedWindow();
}

void EventWatcher::handleAddedWindows()
{
    if (!m_windowAddedCallback) {
        return;
    }
    auto windows = kdk::WindowManager::self()->windows();
    for (auto &wid : qAsConst(windows)) {
        m_windowAddedCallback(wid.toString().toStdString());
    }
}

void EventWatcher::handleActivedWindow()
{
    if (!m_activeWindowChangedCallback) {
        return;
    }

    m_activeWindowChangedCallback(kdk::WindowManager::self()->currentActiveWindow().toString().toStdString());
}

