/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTCHARACTERISTICINTERFACE_H
#define GATTCHARACTERISTICINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>
#include <QStringList>

class QDBusObjectPath;

class GattCharacteristicObject : public QObject
{
public:
    explicit GattCharacteristicObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class GattCharacteristicInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattCharacteristic1")
    Q_PROPERTY(QString UUID READ UUID)
    Q_PROPERTY(QDBusObjectPath Service READ service)
    Q_PROPERTY(QByteArray Value READ value)
    Q_PROPERTY(bool Notifying READ notifying)
    Q_PROPERTY(QStringList Flags READ flags)
    Q_PROPERTY(quint16 Handle READ handle WRITE setHandle)
    Q_PROPERTY(quint16 MTU READ MTU)

public:
    explicit GattCharacteristicInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString UUID() const;

    QDBusObjectPath service() const;

    QByteArray value() const;

    bool notifying() const;

    QStringList flags() const;

    quint16 handle() const;
    void setHandle(const quint16 handle);

    quint16 MTU() const;

public Q_SLOTS:
    QByteArray ReadValue(const QVariantMap& options);
    void WriteValue(const QByteArray& value, const QVariantMap& options);
    void StartNotify();
    void StopNotify();

private:

    friend class DeviceManager;
};

#endif // GATTCHARACTERISTICINTERFACE_H
