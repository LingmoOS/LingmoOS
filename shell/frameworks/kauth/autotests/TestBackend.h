/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TEST_BACKEND_H
#define TEST_BACKEND_H

#include "AuthBackend.h"
#include <QHash>

class QByteArray;

namespace KAuth
{
class TestBackend : public AuthBackend
{
    Q_OBJECT
    Q_INTERFACES(KAuth::AuthBackend)

public:
    TestBackend();
    void setupAction(const QString &) override;
    Action::AuthStatus authorizeAction(const QString &) override;
    Action::AuthStatus actionStatus(const QString &) override;
    QByteArray callerID() const override;
    bool isCallerAuthorized(const QString &action, const QByteArray &callerID, const QVariantMap &details) override;

public Q_SLOTS:
    void setNewCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
    QHash<QString, KAuth::Action::AuthStatus> m_actionStatuses;
};

} // namespace Auth

#endif
