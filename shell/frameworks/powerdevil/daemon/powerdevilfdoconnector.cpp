/*
 *   SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>
 *   SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilfdoconnector.h"

#include "powerdevilaction.h"
#include "powerdevilcore.h"

#include "powermanagementfdoadaptor.h"
#include "powermanagementinhibitadaptor.h"

#include <KConfigGroup>

using namespace Qt::StringLiterals;

namespace PowerDevil
{
FdoConnector::FdoConnector(PowerDevil::Core *parent)
    : QObject(parent)
    , m_core(parent)
{
    new PowerManagementFdoAdaptor(this);
    new PowerManagementInhibitAdaptor(this);

    QDBusConnection c = QDBusConnection::sessionBus();

    c.registerService(u"org.freedesktop.PowerManagement"_s);
    c.registerObject(u"/org/freedesktop/PowerManagement"_s, this);

    c.registerService(u"org.freedesktop.PowerManagement.Inhibit"_s);
    c.registerObject(u"/org/freedesktop/PowerManagement/Inhibit"_s, this);

    connect(m_core->batteryController(), &BatteryController::acAdapterStateChanged, this, &FdoConnector::onAcAdapterStateChanged);
    connect(PolicyAgent::instance(),
            SIGNAL(unavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies)),
            this,
            SLOT(onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies)));
}

bool FdoConnector::CanHibernate()
{
    return m_core->suspendController()->canHibernate();
}

bool FdoConnector::CanSuspend()
{
    return m_core->suspendController()->canSuspend();
}

bool FdoConnector::CanHybridSuspend()
{
    return m_core->suspendController()->canHybridSuspend();
}

bool FdoConnector::CanSuspendThenHibernate()
{
    return m_core->suspendController()->canSuspendThenHibernate();
}

bool FdoConnector::GetPowerSaveStatus()
{
    return m_core->batteryController()->acAdapterState() == BatteryController::Unplugged;
}

void FdoConnector::Suspend()
{
    triggerSuspendSession(PowerButtonAction::Sleep);
}

void FdoConnector::Hibernate()
{
    triggerSuspendSession(PowerButtonAction::Hibernate);
}

bool FdoConnector::HasInhibit()
{
    return PolicyAgent::instance()->requirePolicyCheck(PolicyAgent::InterruptSession) != PolicyAgent::None;
}

int FdoConnector::Inhibit(const QString &application, const QString &reason)
{
    // Inhibit here means we cannot interrupt the session.
    // If we've been called from DBus, use PolicyAgent's service watching system
    if (calledFromDBus()) {
        return PolicyAgent::instance()->addInhibitionWithExplicitDBusService((uint)PolicyAgent::InterruptSession, application, reason, message().service());
    } else {
        return PolicyAgent::instance()->AddInhibition((uint)PolicyAgent::InterruptSession, application, reason);
    }
}

void FdoConnector::UnInhibit(int cookie)
{
    PolicyAgent::instance()->ReleaseInhibition(cookie);
}

void FdoConnector::ForceUnInhibitAll()
{
    PolicyAgent::instance()->releaseAllInhibitions();
}

void FdoConnector::onAcAdapterStateChanged(BatteryController::AcAdapterState newstate)
{
    Q_EMIT PowerSaveStatusChanged(newstate == BatteryController::Unplugged);
}

void FdoConnector::onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies newpolicies)
{
    Q_EMIT HasInhibitChanged(newpolicies & PowerDevil::PolicyAgent::InterruptSession);
}

void FdoConnector::triggerSuspendSession(PowerButtonAction action)
{
    PowerDevil::Action *helperAction = m_core->action(u"SuspendSession"_s);
    if (helperAction) {
        QVariantMap args;
        args[u"Type"_s] = qToUnderlying(action);
        args[u"Explicit"_s] = true;
        helperAction->trigger(args);
    }
}

}

#include "moc_powerdevilfdoconnector.cpp"
