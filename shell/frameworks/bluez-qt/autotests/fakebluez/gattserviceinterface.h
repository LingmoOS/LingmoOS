/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTSERVICEINTERFACE_H
#define GATTSERVICEINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>
#include <QStringList>

class QDBusObjectPath;

class GattServiceObject : public QObject
{
public:
    explicit GattServiceObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class GattServiceInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattService1")
    Q_PROPERTY(QString UUID READ UUID)
    Q_PROPERTY(bool Primary READ primary)
    Q_PROPERTY(QDBusObjectPath Device READ device)
    Q_PROPERTY(QList<QDBusObjectPath> Includes READ includes)
    Q_PROPERTY(quint16 Handle READ handle WRITE setHandle)

public:
    explicit GattServiceInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString UUID() const;

    bool primary() const;

    QDBusObjectPath device() const;

    QList<QDBusObjectPath> includes() const;

    quint16 handle() const;
    void setHandle(const quint16 handle);

private:

    friend class DeviceManager;
};

#endif // GATTSERVICEINTERFACE_H
