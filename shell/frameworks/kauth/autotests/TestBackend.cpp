/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "TestBackend.h"

#include <QDebug>

namespace KAuth
{
TestBackend::TestBackend()
    : AuthBackend()
{
    qDebug() << "Test backend loaded";
    setCapabilities(AuthorizeFromHelperCapability);
}

void TestBackend::setNewCapabilities(AuthBackend::Capabilities capabilities)
{
    qDebug() << "Capabilities changing";
    setCapabilities(capabilities);
}

Action::AuthStatus TestBackend::authorizeAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        return Action::DeniedStatus;
    }

    return Action::AuthorizedStatus;
}

void TestBackend::setupAction(const QString &action)
{
    if (action == QLatin1String("doomed.to.fail")) {
        m_actionStatuses.insert(action, Action::DeniedStatus);
    } else if (action == QLatin1String("requires.auth") || action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::AuthRequiredStatus);
    } else if (action == QLatin1String("always.authorized")) {
        m_actionStatuses.insert(action, Action::AuthorizedStatus);
    } else if (action.startsWith(QLatin1String("org.kde.kf6auth.autotest"))) {
        m_actionStatuses.insert(action, Action::AuthRequiredStatus);
    }
}

Action::AuthStatus TestBackend::actionStatus(const QString &action)
{
    if (m_actionStatuses.contains(action)) {
        return m_actionStatuses.value(action);
    }

    return Action::InvalidStatus;
}

QByteArray TestBackend::callerID() const
{
    return qgetenv("KAUTH_TEST_CALLER_ID");
}

bool TestBackend::isCallerAuthorized(const QString &action, const QByteArray &callerId, const QVariantMap &details)
{
    Q_UNUSED(details);

    if (action == QLatin1String("doomed.to.fail")) {
        return false;
    } else if (action == QLatin1String("requires.auth")) {
        m_actionStatuses.insert(action, Action::AuthorizedStatus);
        Q_EMIT actionStatusChanged(action, Action::AuthorizedStatus);
        return true;
    } else if (action == QLatin1String("generates.error")) {
        m_actionStatuses.insert(action, Action::ErrorStatus);
        Q_EMIT actionStatusChanged(action, Action::ErrorStatus);
        return false;
    } else if (action == QLatin1String("always.authorized")) {
        return true;
    } else if (action.startsWith(QLatin1String("org.kde.kf6auth.autotest"))) {
        qDebug() << "Caller ID:" << callerId << callerID();
        if (callerId == callerID()) {
            m_actionStatuses.insert(action, Action::AuthorizedStatus);
            Q_EMIT actionStatusChanged(action, Action::AuthorizedStatus);
            return true;
        } else {
            m_actionStatuses.insert(action, Action::DeniedStatus);
            Q_EMIT actionStatusChanged(action, Action::DeniedStatus);
        }
    }

    return false;
}

} // namespace Auth

#include "moc_TestBackend.cpp"
