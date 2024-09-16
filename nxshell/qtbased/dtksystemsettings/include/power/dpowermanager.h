// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkpower_global.h"

#include <dexpected.h>
#include <qobject.h>

DPOWER_BEGIN_NAMESPACE
using DCORE_NAMESPACE::DExpected;

class DPowerManagerPrivate;
class DPowerDevice;
class DKbdBacklight;

class DPowerManager : public QObject
{
    Q_OBJECT
public:
    explicit DPowerManager(QObject *parent = nullptr);
    virtual ~DPowerManager();
    // properties
    Q_PROPERTY(bool lidIsClosed READ lidIsClosed NOTIFY lidIsClosedChanged);
    Q_PROPERTY(bool lidIsPresent READ lidIsPresent NOTIFY lidIsPresentChanged);
    Q_PROPERTY(bool hasBattery READ hasBattery);
    Q_PROPERTY(bool onBattery READ onBattery);
    Q_PROPERTY(bool supportKbdBacklight READ supportKbdBacklight);
    Q_PROPERTY(QString daemonVersion READ daemonVersion);
    bool lidIsClosed() const;
    bool lidIsPresent() const;
    bool hasBattery() const;
    bool onBattery() const;
    bool supportKbdBacklight() const;
    QString daemonVersion() const;
    QSharedPointer<DPowerDevice> displayDevice() const;
    QSharedPointer<DPowerDevice> findDeviceByName(const QString &name) const;
    QSharedPointer<DKbdBacklight> kbdBacklight() const;

signals:
    void deviceAdded(const QString &name);
    void deviceRemoved(const QString &name);
    void lidIsClosedChanged(const bool &value);
    void lidIsPresentChanged(const bool &value);

public slots:
    DExpected<QStringList> devices() const;
    DExpected<QString> criticalAction() const;
    DExpected<void> refresh();

private:
    QScopedPointer<DPowerManagerPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DPowerManager)
};

DPOWER_END_NAMESPACE
