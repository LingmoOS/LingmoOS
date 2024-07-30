/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_MEDIATRANSPORT_H
#define BLUEZQT_MEDIATRANSPORT_H

#include <QObject>

#include "bluezqt_export.h"
#include "mediatypes.h"
#include "tpendingcall.h"

#include <memory>

namespace BluezQt
{
class PendingCall;

/**
 * @class BluezQt::MediaTransport mediatransport.h <BluezQt/MediaTransport>
 *
 * Media transport.
 *
 * This class represents a media transport interface.
 */
class BLUEZQT_EXPORT MediaTransport : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(quint16 volume READ volume NOTIFY volumeChanged)

public:
    /** Indicates the state of the transport. */
    enum class State {
        Idle,
        Pending,
        Active,
    };
    Q_ENUM(State)

    /**
     * Destroys a MediaTransport object.
     */
    ~MediaTransport() override;

    /**
     * Returns the (audio) configuration of the transport.
     *
     * @return configuration of transport
     */
    AudioConfiguration audioConfiguration() const;

    /**
     * Returns the state of the transport.
     *
     * @return state of transport
     */
    State state() const;

    /**
     * Returns the volume of the transport.
     *
     * The volume is a percentage of the maximum. The value 0x00 corresponds to 0%.
     * The value 0x7F corresponds to 100%. Scaling should be applied to achieve
     * values between these two. The existence of this scale does not impose any
     * restriction on the granularity of the volume control scale on the target.
     * As this command specifies a percentage rather than an absolute dB level
     * the controller should exercise caution when sending this command.
     *
     * @return volume of transport
     */
    quint16 volume() const;

public Q_SLOTS:
    /**
     * Acquire transport file descriptor and the MTU for read
     * and write respectively.
     *
     * Possible errors: PendingCall::NotAuthorized, PendingCall::Failed
     *
     * @return <fd, uint16, uint16> pending call
     */
    TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t> *acquire();

    /**
     * Acquire transport file descriptor only if the transport
     * is in "pending" state at the time the message is
     * received by BlueZ. Otherwise no request will be sent
     * to the remote device and the function will just fail
     * with org.bluez.Error.NotAvailable.
     *
     * Possible errors: PendingCall::NotAuthorized, PendingCall::Failed, PendingCall::NotAvailable
     *
     * @return <fd, uint16, uint16> pending call
     */
    TPendingCall<QDBusUnixFileDescriptor, uint16_t, uint16_t> *tryAcquire();

    /**
     * Releases file descriptor.
     *
     * @return void pending call
     */
    TPendingCall<void> *release();

Q_SIGNALS:
    /**
     * Indicates that transport's state have changed.
     */
    void stateChanged(State state);

    /**
     * Indicates that transport's volume have changed.
     */
    void volumeChanged(quint16 volume);

private:
    BLUEZQT_NO_EXPORT explicit MediaTransport(const QString &path, const QVariantMap &properties);

    std::unique_ptr<class MediaTransportPrivate> const d;

    friend class MediaTransportPrivate;
    friend class DevicePrivate;
};

} // namespace BluezQt

#endif
