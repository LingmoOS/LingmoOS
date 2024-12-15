// SPDX-FileCopyrightText: 2018-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QScopedPointer>
#include "authorize.h"

namespace ddc
{
class SessionPrivate;
class Session: public QObject
{
Q_OBJECT
public:
    explicit Session(QObject *parent = Q_NULLPTR);
    ~Session() Q_DECL_OVERRIDE;

Q_SIGNALS:
    void authorizeFinished(const AuthorizeResponse &);

public Q_SLOTS:
    void authorize(const AuthorizeRequest &authReq);

public:
    void save(const QString &sessionID);
    void clear();

private:
    QScopedPointer<SessionPrivate> dd_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(dd_ptr), Session)
};

}
