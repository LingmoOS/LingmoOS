/*
 * SPDX-FileCopyrightText: 2018-2019 Daniel Vrátil <dvratil@kde.org>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KDED_BOLT_H
#define KDED_BOLT_H

#include "manager.h"

#include <kdedmodule.h>

#include <QList>
#include <QMap>
#include <QSharedPointer>
#include <QTimer>

class KNotification;
namespace Bolt
{
class Device;
}

class Q_DECL_EXPORT KDEDBolt : public KDEDModule
{
    Q_OBJECT

public:
    using BoltDeviceList = QList<QSharedPointer<Bolt::Device>>;

    KDEDBolt(QObject *parent, const QVariantList &args);
    ~KDEDBolt() override;

protected:
    virtual void notify();

    BoltDeviceList sortDevices(const BoltDeviceList &devices);

private:
    enum AuthMode {
        Enroll,
        Authorize,
    };
    void authorizeDevices(BoltDeviceList devices, AuthMode mode);

protected:
    Bolt::Manager mManager;
    BoltDeviceList mPendingDevices;
    QMap<KNotification *, BoltDeviceList> mNotifiedDevices;
    QTimer mPendingDeviceTimer;
};

#endif // KDED_BOLT_H
