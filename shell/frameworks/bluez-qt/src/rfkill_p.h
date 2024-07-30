/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef BLUEZQT_RFKILL_P_H
#define BLUEZQT_RFKILL_P_H

#include <QHash>
#include <QObject>

#include "bluezqt_export.h"
#include "rfkill.h"

namespace BluezQt
{
struct RfkillPrivate {
    int m_readFd = -1;
    int m_writeFd = -1;
    Rfkill::State m_state = Rfkill::State::Unknown;
    QHash<quint32, Rfkill::State> m_devices;
};
} // namespace BluezQt

#endif // BLUEZQT_RFKILL_P_H
