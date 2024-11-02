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

#ifndef EVENTWATCHER_H
#define EVENTWATCHER_H

#include <memory>
#include <functional>
#include <QObject>
#include "schedpolicy.h"
#include "devicestate.h"
#include "daemondbusinterface.h"

class EventWatcher : public QObject
{
    Q_OBJECT

public:
    using WindowChangedCallback = std::function<void(const std::string &)>;
    using PowerModeChangedCallback = std::function<void(sched_policy::PowerMode)>;
    using PowerTypeChangedCallback = std::function<void(sched_policy::PowerType)>;
    using DeviceModeChangedCallback = std::function<void(sched_policy::DeviceMode)>;
    using MprisDbusAddedCallback = std::function<void(int, const std::string &)>;
    using ResourceWarningCallback = std::function<void(const std::string&, int)>;
    using StatusNotifierItemRegisteredCallback = std::function<void(int pid)>;

    EventWatcher(std::unique_ptr<PowerManager> powerManager,
                 std::unique_ptr<DeviceModeManager> deviceModeManager);

    void startWatcher();
    void stopWatcher();

    void setWindowAddedCallback(WindowChangedCallback callback);
    void setWindowRemovedCallback(WindowChangedCallback callback);
    void setActiveWindowChangedCallback(WindowChangedCallback callback);
    void setWindowMinimizedCallback(WindowChangedCallback callback);

    void setPowerModeChangedCallback(PowerModeChangedCallback callback);
    void setPowerTypeChangedCallback(PowerTypeChangedCallback callback);
    void setDeviceModeChangedCallback(DeviceModeChangedCallback callback);

    void setMprisDbusAddedCallback(MprisDbusAddedCallback callback);
    void setStatusNotifierItemRegisteredCallback(StatusNotifierItemRegisteredCallback callback);

    void setResourceWarningCallback(ResourceWarningCallback callback);

    sched_policy::PowerType powerType() const;
    sched_policy::DeviceMode deviceMode() const;

    void handleSoftFreezeModeEnabledChanged(bool enabled);

private Q_SLOTS:
    void onMprisDbusNameAcquired(QString name, QString newOwner, QString oldOwner);
    void onStatusNotifierItemRegistered(QString itemName);

private:
    void initWindowChangedConnections();
    void initPowerModeChangedConnections();
    void initPowerTypeChangedConnections();
    void initDeviceModeChangedConnections();
    void initMprisDbusConnections();
    void initStatusNotifierWatcherConnections();
    void initDaemonInterfaceConnections();
    void handleExistedWindows();
    void handleAddedWindows();
    void handleActivedWindow();

private:
    WindowChangedCallback m_windowAddedCallback;
    WindowChangedCallback m_windowRemovedCallback;
    WindowChangedCallback m_activeWindowChangedCallback;
    WindowChangedCallback m_windowMinimizedCallback;

    PowerModeChangedCallback m_powerModeChangedCallback;
    PowerTypeChangedCallback m_powerTypeChangedCallback;
    DeviceModeChangedCallback m_deviceModeChangedCallback;

    MprisDbusAddedCallback m_mprisDbusAddedCallback;
    StatusNotifierItemRegisteredCallback m_statusNotifierItemRegisteredCallback;

    ResourceWarningCallback m_resourceWarningCallback;

    std::unique_ptr<PowerManager> m_powerManager;
    std::unique_ptr<DeviceModeManager> m_deviceModeManager;

    DaemonDbusInterface m_daemonInterface;
};

#endif // EVENTWATCHER_H
