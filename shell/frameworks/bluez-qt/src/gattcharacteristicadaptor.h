/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_GATTCHARACTERISTICADAPTOR_H
#define BLUEZQT_GATTCHARACTERISTICADAPTOR_H

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

namespace BluezQt
{
class GattCharacteristic;

class GattCharacteristicAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.GattCharacteristic1")
    Q_PROPERTY(QString UUID READ uuid)
    Q_PROPERTY(QDBusObjectPath Service READ service)
    Q_PROPERTY(QStringList Flags READ flags)

public:
    explicit GattCharacteristicAdaptor(GattCharacteristic *parent);

    QString uuid() const;

    QDBusObjectPath service() const;

    QStringList flags() const;

public Q_SLOTS:
    QByteArray ReadValue(const QVariantMap &options);
    void WriteValue(const QByteArray &value, const QVariantMap &options);
    void StartNotify();
    void StopNotify();

private:
    GattCharacteristic *m_gattCharacteristic;
};

} // namespace BluezQt

#endif
