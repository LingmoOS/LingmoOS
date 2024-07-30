/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_INPUT_H
#define BLUEZQT_INPUT_H

#include <QObject>

#include "bluezqt_export.h"
#include "types.h"

#include <memory>

namespace BluezQt
{
/**
 * @class BluezQt::Input input.h <BluezQt/Input>
 *
 * %Device input.
 *
 * This class represents an input interface.
 */
class BLUEZQT_EXPORT Input : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ReconnectMode reconnectMode READ reconnectMode NOTIFY reconnectModeChanged)

public:
    /** Reconnect mode. */
    enum ReconnectMode {
        /** Device and host are not required to automatically restore the connection. */
        NoReconnect,
        /** Host restores the connection. */
        HostReconnect,
        /** Device restores the connection. */
        DeviceReconnect,
        /** Device shall attempt to restore the lost connection, but host may also restore the connection. */
        AnyReconnect,
    };
    Q_ENUM(ReconnectMode)

    /**
     * Destroys an Input object.
     */
    ~Input() override;

    /**
     * Returns a shared pointer from this.
     *
     * @return InputPtr
     */
    InputPtr toSharedPtr() const;

    /**
     * Returns the reconnect mode.
     *
     * @return reconnect mode
     */
    ReconnectMode reconnectMode() const;

Q_SIGNALS:
    /**
     * Indicates that input's reconnect mode have changed.
     */
    void reconnectModeChanged(ReconnectMode mode);

private:
    BLUEZQT_NO_EXPORT explicit Input(const QString &path, const QVariantMap &properties);

    std::unique_ptr<class InputPrivate> const d;

    friend class InputPrivate;
    friend class DevicePrivate;
};

} // namespace BluezQt

#endif // BLUEZQT_INPUT_H
