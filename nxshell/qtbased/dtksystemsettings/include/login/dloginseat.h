// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtklogin_global.h"

#include <dexpected.h>
#include <qobject.h>

DLOGIN_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;
class DLoginSeatPrivate;

class DLoginSeat : public QObject
{
    Q_OBJECT

public:
    ~DLoginSeat() override;

    Q_PROPERTY(QStringList sessions READ sessions);
    Q_PROPERTY(bool canGraphical READ canGraphical);
    Q_PROPERTY(bool canTTY READ canTTY);
    Q_PROPERTY(bool idleHint READ idleHint);
    Q_PROPERTY(QString id READ id);
    Q_PROPERTY(QString activeSession READ activeSession);
    Q_PROPERTY(QDateTime idleSinceHint READ idleSinceHint);
    Q_PROPERTY(quint64 idleSinceHintMonotonic READ idleSinceHintMonotonic);

    bool canGraphical() const;
    bool canTTY() const;
    bool idleHint() const;
    QStringList sessions() const;
    QString id() const;
    QString activeSession() const;
    QDateTime idleSinceHint() const;
    quint64 idleSinceHintMonotonic() const;

public Q_SLOTS:
    DExpected<void> activateSession(const QString &sessionId);
    DExpected<void> switchTo(quint32 VTNr);

private:
    friend class DLoginManager;
    explicit DLoginSeat(const QString &path, QObject *parent = nullptr);
    QScopedPointer<DLoginSeatPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DLoginSeat)
};

DLOGIN_END_NAMESPACE
