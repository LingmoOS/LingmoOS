/*
 *   SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>

#include "controllers/batterycontroller.h"
#include "powerdevilenums.h"
#include "powerdevilpolicyagent.h"

namespace PowerDevil
{
class Core;

class FdoConnector : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_DISABLE_COPY(FdoConnector)

    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PowerManagement")
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.PowerManagement.Inhibit")

public:
    explicit FdoConnector(PowerDevil::Core *parent);

    bool CanHibernate();
    bool CanSuspend();
    bool CanHybridSuspend();
    bool CanSuspendThenHibernate();

    bool GetPowerSaveStatus();

    void Suspend();
    void Hibernate();

    bool HasInhibit();

    int Inhibit(const QString &application, const QString &reason);
    void UnInhibit(int cookie);
    void ForceUnInhibitAll();

Q_SIGNALS:
    void CanSuspendChanged(bool canSuspend);
    void CanHibernateChanged(bool canHibernate);
    void CanHybridSuspendChanged(bool canHybridSuspend);
    void CanSuspendThenHibernateChanged(bool canSuspendThenHibernate);
    void PowerSaveStatusChanged(bool savePower);

    void HasInhibitChanged(bool hasInhibit);

private Q_SLOTS:
    void onAcAdapterStateChanged(BatteryController::AcAdapterState);
    void onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies);
    void triggerSuspendSession(PowerDevil::PowerButtonAction action);

private:
    PowerDevil::Core *m_core;
};

}
