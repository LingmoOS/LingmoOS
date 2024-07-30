/*
 * SPDX-FileCopyrightText: 2021 Ivan Podkurkov <podkiva2@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTDESCRIPTORINTERFACE_H
#define GATTDESCRIPTORINTERFACE_H

#include "object.h"

#include <QDBusAbstractAdaptor>
#include <QStringList>

class QDBusObjectPath;

class GattDescriptorObject : public QObject
{
public:
    explicit GattDescriptorObject(const QDBusObjectPath &path, QObject *parent = nullptr);
};

class GattDescriptorInterface : public QDBusAbstractAdaptor, public Object
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattDescriptor1")
    Q_PROPERTY(QString UUID READ UUID)
    Q_PROPERTY(QDBusObjectPath Characteristic READ characteristic)
    Q_PROPERTY(QByteArray Value READ value)
    Q_PROPERTY(QStringList Flags READ flags)
    Q_PROPERTY(quint16 Handle READ handle WRITE setHandle)

public:
    explicit GattDescriptorInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent = nullptr);

    QString UUID() const;

    QDBusObjectPath characteristic() const;

    QByteArray value() const;

    QStringList flags() const;

    quint16 handle() const;
    void setHandle(const quint16 handle);

public Q_SLOTS:
    QByteArray ReadValue(const QVariantMap& options);
    void WriteValue(const QByteArray& value, const QVariantMap& options);

private:

    friend class DeviceManager;
};

#endif // GATTDESCRIPTORINTERFACE_H
