/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_RFKILL_H
#define BLUEZQT_RFKILL_H

#include <QHash>
#include <QObject>

#include "bluezqt_export.h"

#include <memory>

namespace BluezQt
{
struct RfkillPrivate;

class BLUEZQT_EXPORT Rfkill : public QObject
{
    Q_OBJECT
    Q_PROPERTY(State state READ state NOTIFY stateChanged)

public:
    enum State {
        Unblocked = 0,
        SoftBlocked = 1,
        HardBlocked = 2,
        Unknown = 3,
    };
    Q_ENUM(State)

    explicit Rfkill(QObject *parent = nullptr);
    ~Rfkill() override;

    State state() const;
    void block();
    void unblock();

Q_SIGNALS:
    void stateChanged(State state);

private Q_SLOTS:
    BLUEZQT_NO_EXPORT void devReadyRead();

private:
    BLUEZQT_NO_EXPORT void init();
    BLUEZQT_NO_EXPORT bool openForWriting();
    BLUEZQT_NO_EXPORT void updateRfkillDevices();
    BLUEZQT_NO_EXPORT bool setSoftBlock(quint8 soft);

    std::unique_ptr<RfkillPrivate> d;
};

} // namespace BluezQt

#endif // BLUEZQT_RFKILL_H
