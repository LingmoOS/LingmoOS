/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009 Radek Novacek <rnovacek@redhat.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef POLKIT1BACKEND_H
#define POLKIT1BACKEND_H

#include "AuthBackend.h"

#include <QEventLoop>
#include <QHash>
#include <QStringList>

#include <PolkitQt1/Authority>

#include <memory>

class QByteArray;

namespace KAuth
{
class Polkit1Backend : public AuthBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.Polkit1Backend")
    Q_INTERFACES(KAuth::AuthBackend)

public:
    Polkit1Backend();
    ~Polkit1Backend() override;
    void setupAction(const QString &) override;
    void preAuthAction(const QString &action, QWindow *parent) override;
    Action::AuthStatus authorizeAction(const QString &) override;
    Action::AuthStatus actionStatus(const QString &) override;
    QByteArray callerID() const override;
    virtual bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details) override;
    QVariantMap backendDetails(const DetailsMap &details) override;

private Q_SLOTS:
    void checkForResultChanged();

private:
    void sendWindowHandle(const QString &action, const QString &handle);
    void sendActivationToken(const QString &action, QWindow *window);

    QHash<QString, Action::AuthStatus> m_cachedResults;
};

} // namespace Auth

#endif
