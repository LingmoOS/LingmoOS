/*
 *   SPDX-FileCopyrightText: 2012 Daniel Nicoletti <dantti12@gmail.com>
 *   SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"

#include <QObject>
#include <QQuaternion>
#include <QtGlobal>

namespace KScreen
{
class KSCREEN_EXPORT Edid : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString deviceId READ deviceId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString vendor READ vendor CONSTANT)
    Q_PROPERTY(QString serial READ serial CONSTANT)
    Q_PROPERTY(QString eisaId READ eisaId CONSTANT)
    Q_PROPERTY(QString hash READ hash CONSTANT)
    Q_PROPERTY(uint width READ width CONSTANT)
    Q_PROPERTY(uint height READ height CONSTANT)
    Q_PROPERTY(qreal gamma READ gamma CONSTANT)
    Q_PROPERTY(QQuaternion red READ red CONSTANT)
    Q_PROPERTY(QQuaternion green READ green CONSTANT)
    Q_PROPERTY(QQuaternion blue READ blue CONSTANT)
    Q_PROPERTY(QQuaternion white READ white CONSTANT)

public:
    explicit Edid();
    explicit Edid(const QByteArray &data, QObject *parent = nullptr);
    ~Edid() override;

    Q_REQUIRED_RESULT Edid *clone() const;

    bool isValid() const;

    QString deviceId(const QString &fallbackName = QString()) const;
    QString name() const;
    QString vendor() const;
    QString serial() const;
    QString eisaId() const;
    QString hash() const;
    QString pnpId() const;
    uint width() const;
    uint height() const;
    qreal gamma() const;
    QQuaternion red() const;
    QQuaternion green() const;
    QQuaternion blue() const;
    QQuaternion white() const;

private:
    Q_DISABLE_COPY(Edid)

    class Private;
    Private *const d;

    explicit Edid(Private *dd);
};

}
