/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef LESERVER_H
#define LESERVER_H

#include <QObject>
#include <QTimer>

#include "gattcharacteristic.h"
#include "types.h"

namespace BluezQt
{
class PendingCall;
}

class LeServer : public QObject
{
    Q_OBJECT

public:
    explicit LeServer(BluezQt::Manager *manager, QObject *parent = nullptr);

private:
    void onCallFinished(BluezQt::PendingCall *call);

    BluezQt::Manager *m_manager;
    BluezQt::GattCharacteristic *m_notifyingCharacteristic = nullptr;

    QTimer m_characteristicWriteTimer; // < Used to write to characteristic with 'notify' flag set.
};

#endif
