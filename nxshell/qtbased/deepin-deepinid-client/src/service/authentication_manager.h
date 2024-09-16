// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>

#include "authorize.h"

namespace ddc
{

class AuthenticationManagerPrivate;
class AuthenticationManager: public QObject
{
Q_OBJECT
public:
    explicit AuthenticationManager(QObject *parent = Q_NULLPTR);
    ~AuthenticationManager() Q_DECL_OVERRIDE;

    bool hasRequest() const;
    void cancel();
    void delAuthReq(const QString &clientID);

Q_SIGNALS:
    void authorizeFinished(const AuthorizeResponse &);
    void requestLogin(const AuthorizeRequest &authReq);

public Q_SLOTS:
    void requestAuthorize(const AuthorizeRequest &authReq);
    bool onLogin(const QString &sessionID,
                 const QString &clientID,
                 const QString &code,
                 const QString &state);

private:
    QScopedPointer<AuthenticationManagerPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), AuthenticationManager)
};

}
