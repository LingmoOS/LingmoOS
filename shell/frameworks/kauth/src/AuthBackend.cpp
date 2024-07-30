/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "AuthBackend.h"

namespace KAuth
{
class AuthBackend::Private
{
public:
    Private()
    {
    }
    virtual ~Private()
    {
    }

    Capabilities capabilities;
};

AuthBackend::AuthBackend()
    : QObject(nullptr)
    , d(new Private)
{
}

AuthBackend::~AuthBackend()
{
    delete d;
}

AuthBackend::Capabilities AuthBackend::capabilities() const
{
    return d->capabilities;
}

void AuthBackend::setCapabilities(AuthBackend::Capabilities capabilities)
{
    d->capabilities = capabilities;
}

void AuthBackend::preAuthAction(const QString &action, QWindow *parent)
{
    Q_UNUSED(action)
    Q_UNUSED(parent)
}

QVariantMap AuthBackend::backendDetails(const DetailsMap &details)
{
    Q_UNUSED(details);
    return QVariantMap();
}

} // namespace KAuth

#include "moc_AuthBackend.cpp"
