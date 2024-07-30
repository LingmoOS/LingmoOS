/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>
    SPDX-FileCopyrightText: 2009-2010 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KAUTH_AUTH_BACKEND_H
#define KAUTH_AUTH_BACKEND_H

#include <QObject>

#include "action.h"

namespace KAuth
{
typedef Action::DetailsMap DetailsMap;

class AuthBackend : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AuthBackend)

public:
    enum Capability {
        NoCapability = 0,
        AuthorizeFromClientCapability = 1,
        AuthorizeFromHelperCapability = 2,
        PreAuthActionCapability = 4,
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    AuthBackend();
    ~AuthBackend() override;
    virtual void setupAction(const QString &action) = 0;
    virtual void preAuthAction(const QString &action, QWindow *parent);
    virtual Action::AuthStatus authorizeAction(const QString &action) = 0;
    virtual Action::AuthStatus actionStatus(const QString &action) = 0;
    virtual QByteArray callerID() const = 0;
    virtual bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details) = 0;
    virtual QVariantMap backendDetails(const DetailsMap &details);

    Capabilities capabilities() const;

protected:
    void setCapabilities(Capabilities capabilities);

Q_SIGNALS:
    void actionStatusChanged(const QString &action, KAuth::Action::AuthStatus status);

private:
    class Private;
    Private *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AuthBackend::Capabilities)

} // namespace Auth

Q_DECLARE_INTERFACE(KAuth::AuthBackend, "org.kde.kf6auth.AuthBackend/0.1")

#endif
