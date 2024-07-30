/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2022 Pontus Sjögren
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef GATTDESCRIPTORADAPTOR_H
#define GATTDESCRIPTORADAPTOR_H

#include <QObject>
#include <QStringList>

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

namespace BluezQt
{

class GattDescriptor;

class GattDescriptorAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattDescriptor1")
    Q_PROPERTY(QString UUID READ uuid)
    Q_PROPERTY(QDBusObjectPath Characteristic READ characteristic)
    Q_PROPERTY(QByteArray Value READ value)
    Q_PROPERTY(QStringList Flags READ flags)

public:
    explicit GattDescriptorAdaptor(GattDescriptor *parent);

    QString uuid() const;
    QDBusObjectPath characteristic() const;
    QByteArray value() const;
    QStringList flags() const;

public Q_SLOTS:

    QByteArray ReadValue(const QVariantMap &options);
    void WriteValue(QByteArray value);

private:
    GattDescriptor *m_gattDescriptor;
};

}

#endif
