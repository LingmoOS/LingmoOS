/*
    This file is part of the KDE Project
    SPDX-FileCopyrightText: 2008 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2012-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef BALOO_FILEINDEXER_POWER_STATE_MONITOR_H_
#define BALOO_FILEINDEXER_POWER_STATE_MONITOR_H_

#include <QObject>

namespace Baloo
{

class PowerStateMonitor : public QObject
{
    Q_OBJECT

public:
    explicit PowerStateMonitor(QObject* parent = nullptr);

    bool isOnBattery()    const {
        return m_isOnBattery;
    }

Q_SIGNALS:
    /**
     * Emitted when the power management status changes.
     *
     * \param conserveResources true if you should conserve resources
     */
    void powerManagementStatusChanged(bool conserveResources);

private Q_SLOTS:
    void slotPowerManagementStatusChanged(bool conserveResources);

private:
    bool m_enabled;
    bool m_isOnBattery;
};
}

#endif
