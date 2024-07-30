/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "FakeBackend.h"

namespace KAuth
{
FakeBackend::FakeBackend()
    : AuthBackend()
{
    setCapabilities(NoCapability);
}

Action::AuthStatus FakeBackend::authorizeAction(const QString &action)
{
    Q_UNUSED(action)
    return Action::DeniedStatus;
}

void FakeBackend::setupAction(const QString &action)
{
    Q_UNUSED(action)
}

Action::AuthStatus FakeBackend::actionStatus(const QString &action)
{
    Q_UNUSED(action)
    return Action::DeniedStatus;
}

QByteArray FakeBackend::callerID() const
{
    return QByteArray();
}

bool FakeBackend::isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details)
{
    Q_UNUSED(action)
    Q_UNUSED(callerID)
    Q_UNUSED(details)
    return false;
}

} // namespace Auth

#include "moc_FakeBackend.cpp"
