/*
    SPDX-FileCopyrightText: 2006 Michaël Larouche <michael.larouche@kdemail.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H
#define SOLID_BACKENDS_FAKEHW_FAKEDEVICE_P_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

#include "fakedevice.h"

namespace Solid
{
namespace Backends
{
namespace Fake
{
class FakeDevice;
class FakeDevice::Private : public QObject
{
    Q_OBJECT
public:
    QString udi;
    QMap<QString, QVariant> propertyMap;
    QStringList interfaceList;
    bool locked;
    QString lockReason;
    bool broken;

Q_SIGNALS:
    void propertyChanged(const QMap<QString, int> &changes);
    void conditionRaised(const QString &condition, const QString &reason);

    friend class FakeDevice;
};
}
}
}

#endif
