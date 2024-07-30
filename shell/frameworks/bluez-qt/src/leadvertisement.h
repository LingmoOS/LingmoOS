/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_LEADVERTISEMENT_H
#define BLUEZQT_LEADVERTISEMENT_H

#include <QObject>

#include "bluezqt_export.h"

#include <memory>

class QDBusObjectPath;

namespace BluezQt
{
/**
 * @class BluezQt::LEAdvertisement leadvertisement.h <BluezQt/LEAdvertisement>
 *
 * Bluetooth LE advertisement.
 *
 * This class represents a Bluetooth LE advertisement.
 */
class BLUEZQT_EXPORT LEAdvertisement : public QObject
{
    Q_OBJECT

public:
    /**
     * Creates a new LEAdvertisement object.
     *
     * @param parent
     */
    explicit LEAdvertisement(const QStringList &serviceUuids, QObject *parent = nullptr);

    /**
     * Destroys a LEAdvertisement object.
     */
    ~LEAdvertisement() override;

    /**
     * List of UUIDs to include in the "Service UUID" field of the Advertising Data.
     *
     * @return UUIDs of the advertisement
     */
    virtual QStringList serviceUuids() const;

    /**
     * Returns the service data included in the advertisement.
     *
     * @since 5.75
     */
    QHash<QString, QByteArray> serviceData() const;

    /**
     * Sets the service data to include in the advertisement.
     * Keys are the UUIDs of the associated data.
     *
     * @since 5.75
     */
    void setServiceData(const QHash<QString, QByteArray> &data);

    /**
     * Returns the manufacturer data included in the advertisement.
     *
     * @return A QHash representing the manufaturer IDs and associated data. Keys are manufaturer ID.
     * @since 6.1
     */
    QHash<quint16, QByteArray> manufacturerData() const;

    /**
     * Sets the manufacturer data to be included in the advertisement.
     *
     * @param data QHash representing the manufacturer IDs and associated data. Keys are manufacturer ID.
     * @since 6.1
     */
    void setManufacturerData(const QHash<quint16, QByteArray> &data);

    /**
     * Indicates that the LEAdvertisement was unregistered.
     *
     * This method gets called when the service daemon removes the Advertisement.
     * A client can use it to do cleanup tasks. There is no need to call
     * UnregisterAdvertisement because when this method gets called it has
     * already been unregistered.
     */
    virtual void release();

protected:
    /**
     * D-Bus object path of the advertisement.
     *
     * The path where the advertisement will be registered.
     *
     * @note You must provide valid object path!
     *
     * @return object path of advertisement
     */
    virtual QDBusObjectPath objectPath() const;

private:
    std::unique_ptr<class LEAdvertisementPrivate> const d;

    friend class LEAdvertisingManager;
};

} // namespace BluezQt

#endif // BLUEZQT_LEADVERTISEMENT_H
