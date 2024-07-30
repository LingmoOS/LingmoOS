/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilaction.h"

#include "powerdevil_debug.h"
#include "powerdevilcore.h"

#include <QDebug>

namespace PowerDevil
{

Action::Action(QObject *parent)
    : QObject(parent)
{
    m_core = qobject_cast<PowerDevil::Core *>(parent);
}

Action::~Action()
{
}

void Action::registerIdleTimeout(std::chrono::milliseconds timeout)
{
    m_registeredIdleTimeouts.append(timeout);
    m_core->registerActionTimeout(this, timeout);
}

void Action::unregisterIdleTimeouts()
{
    // Remove all registered idle timeouts, if any
    m_core->unregisterActionTimeouts(this);
    m_registeredIdleTimeouts.clear();
}

void Action::unloadAction()
{
    unregisterIdleTimeouts();
}

bool Action::isSupported()
{
    return true;
}

Core *Action::core() const
{
    return m_core;
}

void Action::trigger(const QVariantMap &args)
{
    if (args.contains(QStringLiteral("Explicit")) && args[QStringLiteral("Explicit")].toBool()) {
        // The action was explicitly triggered by the user, hence any policy check is bypassed.
        triggerImpl(args);
    } else {
        // The action was taken automatically: let's check if we have the rights to do that
        PolicyAgent::RequiredPolicies unsatisfiablePolicies = PolicyAgent::instance()->requirePolicyCheck(m_requiredPolicies);
        if (unsatisfiablePolicies == PolicyAgent::None) {
            // Ok, let's trigger the action
            triggerImpl(args);
        } else {
            // TODO: Notify somehow?
            qCWarning(POWERDEVIL) << "Unsatisfied policies, the action has been aborted";
        }
    }
}

void Action::setRequiredPolicies(PolicyAgent::RequiredPolicies requiredPolicies)
{
    m_requiredPolicies = requiredPolicies;
}

void Action::onIdleTimeout(std::chrono::milliseconds /*timeout*/)
{
}

void Action::onWakeupFromIdle()
{
}

void Action::onProfileLoad(const QString & /*previousProfile*/, const QString & /*newProfile*/)
{
}

void Action::onProfileUnload()
{
}

} // namespace PowerDevil

#include "moc_powerdevilaction.cpp"
