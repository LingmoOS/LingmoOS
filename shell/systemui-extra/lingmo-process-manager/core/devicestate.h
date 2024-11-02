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

#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <QObject>
#include <QDBusInterface>
#include <QGSettings/qgsettings.h>
#include <memory>
#include "schedpolicy.h"

class PowerManager : public QObject
{
    Q_OBJECT

public:
    PowerManager();

    sched_policy::PowerMode currentPowerMode() const;
    sched_policy::PowerType currentPowerType() const;

Q_SIGNALS:
    void powerModeChanged(sched_policy::PowerMode powerMode);
    void powerTypeChanged(sched_policy::PowerType powerType);

private Q_SLOTS:
    void onPropertiesChanged(
        QString name, QMap<QString, QVariant> value, QStringList);

private:
    void initGSettings();
    void initDbusConnection();
    void initPowerMode();
    void handlePowerModeChanged(int value);

private:
    sched_policy::PowerType m_currentPowerType;
    sched_policy::PowerMode m_currentPowerMode;

    std::unique_ptr<QGSettings> m_gsettings;
};

class DeviceModeManager : public QObject
{
    Q_OBJECT

public:
    DeviceModeManager(bool softFreezeEnabled);

    sched_policy::DeviceMode currentDeviceMode() const;

    void handleSoftFreezeModeEnabledChanged(bool enabled);

Q_SIGNALS:
    void deviceModeChanged(sched_policy::DeviceMode mode);

private Q_SLOTS:
    void updateDeviceMode();

private:
    void resetDeviceMode();
    void initConnections();

    bool isTabletMode();

private:
    bool m_softFreezeModeEnabled;
    sched_policy::DeviceMode m_currentDeviceMode;
};

#endif // DEVICESTATE_H
