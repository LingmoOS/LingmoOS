// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <ddbusinterface.h>
#include <qdbuspendingreply.h>
#include <qscopedpointer.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DDBusInterface;
class DKbdBacklightPrivate;

class UPowerKbdBacklightInterface : public QObject
{
    Q_OBJECT
public:
    explicit UPowerKbdBacklightInterface(QObject *parent = nullptr);
    virtual ~UPowerKbdBacklightInterface();

signals:
    void BrightnessChanged(const qint32 value);
    void BrightnessChangedWithSource(const qint32 value, const QString &source);

public slots:
    QDBusPendingReply<qint32> getBrightness() const;
    QDBusPendingReply<qint32> getMaxBrightness() const;
    QDBusPendingReply<> setBrightness(qint32 value);

private:
    DDBusInterface *m_inter;
};

DPOWER_END_NAMESPACE
