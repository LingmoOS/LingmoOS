/*
    SPDX-FileCopyrightText: 2008 Nicola Gigante <nicola.gigante@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef FAKE_BACKEND_H
#define FAKE_BACKEND_H

#include "AuthBackend.h"
#include <QHash>

class QByteArray;

namespace KAuth
{
class FakeBackend : public AuthBackend
{
    Q_OBJECT
    Q_INTERFACES(KAuth::AuthBackend)

public:
    FakeBackend();
    void setupAction(const QString &) override;
    Action::AuthStatus authorizeAction(const QString &) override;
    Action::AuthStatus actionStatus(const QString &) override;
    QByteArray callerID() const override;
    bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details) override;
};

} // namespace Auth

#endif
