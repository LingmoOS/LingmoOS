/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_LEADVERTISEMENTADAPTOR_H
#define BLUEZQT_LEADVERTISEMENTADAPTOR_H

#include <QDBusAbstractAdaptor>
#include <QDBusVariant>

namespace BluezQt
{
class LEAdvertisement;

class LEAdvertisementAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.bluez.LEAdvertisement1")
    Q_PROPERTY(QString Type READ type)
    Q_PROPERTY(QStringList ServiceUUIDs READ serviceUuids)
    Q_PROPERTY(QHash<QString, QVariant> ServiceData READ serviceData)
    Q_PROPERTY(QHash<quint16, QDBusVariant> ManufacturerData READ manufacturerData)

public:
    explicit LEAdvertisementAdaptor(LEAdvertisement *parent);

    QString type() const;

    QStringList serviceUuids() const;
    QHash<QString, QVariant> serviceData() const;
    QHash<quint16, QDBusVariant> manufacturerData() const;

public Q_SLOTS:
    Q_NOREPLY void Release();

private:
    LEAdvertisement *m_advertisement;
};

} // namespace BluezQt

#endif // BLUEZQT_LEADVERTISEMENTADAPTOR_H
