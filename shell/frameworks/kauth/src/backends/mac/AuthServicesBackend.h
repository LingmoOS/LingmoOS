/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef AUTHSERVICES_BACKEND_H
#define AUTHSERVICES_BACKEND_H

#include "AuthBackend.h"
#include <Security/Security.h>

namespace KAuth
{
class AuthServicesBackend : public AuthBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.AuthServicesBackend")
    Q_INTERFACES(KAuth::AuthBackend)

public:
    AuthServicesBackend();
    virtual ~AuthServicesBackend();
    virtual void setupAction(const QString &);
    virtual Action::AuthStatus authorizeAction(const QString &);
    virtual Action::AuthStatus actionStatus(const QString &);
    virtual QByteArray callerID() const;
    virtual bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details);
};

} // namespace KAuth

#endif
