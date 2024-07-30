/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "rfkill.h"
#include "rfkill_p.h"

#include "debug.h"

#ifdef Q_OS_LINUX
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#endif

#include <QSocketNotifier>

namespace BluezQt
{
#ifdef Q_OS_LINUX
enum rfkill_type {
    RFKILL_TYPE_ALL = 0,
    RFKILL_TYPE_WLAN,
    RFKILL_TYPE_BLUETOOTH,
    RFKILL_TYPE_UWB,
    RFKILL_TYPE_WIMAX,
    RFKILL_TYPE_WWAN,
};

enum rfkill_operation {
    RFKILL_OP_ADD = 0,
    RFKILL_OP_DEL,
    RFKILL_OP_CHANGE,
    RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
    quint32 idx;
    quint8 type;
    quint8 op;
    quint8 soft;
    quint8 hard;
};
#endif

Rfkill::Rfkill(QObject *parent)
    : QObject(parent)
    , d(new RfkillPrivate)
{
    init();
}

Rfkill::~Rfkill()
{
#ifdef Q_OS_LINUX
    if (d->m_readFd != -1) {
        ::close(d->m_readFd);
    }

    if (d->m_writeFd != -1) {
        ::close(d->m_writeFd);
    }
#endif
}

Rfkill::State Rfkill::state() const
{
    return d->m_state;
}

void Rfkill::block()
{
    if (d->m_state == SoftBlocked || d->m_state == HardBlocked) {
        return;
    }

    setSoftBlock(1);
}

void Rfkill::unblock()
{
    if (d->m_state == Unblocked) {
        return;
    }

    setSoftBlock(0);
}

void Rfkill::devReadyRead()
{
    State oldState = d->m_state;

    updateRfkillDevices();

    if (d->m_state != oldState) {
        Q_EMIT stateChanged(d->m_state);
    }
}

void Rfkill::init()
{
#ifdef Q_OS_LINUX
    d->m_readFd = ::open("/dev/rfkill", O_RDONLY | O_CLOEXEC);

    if (d->m_readFd == -1) {
        qCWarning(BLUEZQT) << "Cannot open /dev/rfkill for reading!";
        return;
    }

    if (::fcntl(d->m_readFd, F_SETFL, O_NONBLOCK) < 0) {
        ::close(d->m_readFd);
        d->m_readFd = -1;
        return;
    }

    updateRfkillDevices();

    QSocketNotifier *notifier = new QSocketNotifier(d->m_readFd, QSocketNotifier::Read, this);
    connect(notifier, &QSocketNotifier::activated, this, &Rfkill::devReadyRead);
#endif
}

bool Rfkill::openForWriting()
{
#ifndef Q_OS_LINUX
    return false;
#else
    if (d->m_writeFd != -1) {
        return true;
    }

    d->m_writeFd = ::open("/dev/rfkill", O_WRONLY | O_CLOEXEC);

    if (d->m_writeFd == -1) {
        qCWarning(BLUEZQT) << "Cannot open /dev/rfkill for writing!";
        return false;
    }

    if (::fcntl(d->m_writeFd, F_SETFL, O_NONBLOCK) < 0) {
        ::close(d->m_writeFd);
        d->m_writeFd = -1;
        return false;
    }

    return true;
#endif
}

#ifdef Q_OS_LINUX
static Rfkill::State getState(const rfkill_event &event)
{
    if (event.hard) {
        return Rfkill::HardBlocked;
    } else if (event.soft) {
        return Rfkill::SoftBlocked;
    }
    return Rfkill::Unblocked;
}
#endif

void Rfkill::updateRfkillDevices()
{
#ifdef Q_OS_LINUX
    if (d->m_readFd == -1) {
        return;
    }

    rfkill_event event;
    while (::read(d->m_readFd, &event, sizeof(event)) == sizeof(event)) {
        if (event.type != RFKILL_TYPE_BLUETOOTH) {
            continue;
        }

        switch (event.op) {
        case RFKILL_OP_ADD:
        case RFKILL_OP_CHANGE:
            d->m_devices[event.idx] = getState(event);
            break;

        case RFKILL_OP_DEL:
            d->m_devices.remove(event.idx);
            break;

        case RFKILL_OP_CHANGE_ALL:
            for (auto it = d->m_devices.begin(); it != d->m_devices.end(); ++it) {
                it.value() = getState(event);
            }
            break;

        default:
            break;
        }
    }

    // Update global state
    d->m_state = Unknown;

    for (State state : std::as_const(d->m_devices)) {
        Q_ASSERT(state != Unknown);

        if (d->m_state == Unknown) {
            d->m_state = state;
        } else if (state > d->m_state) {
            d->m_state = state;
        }
    }

    qCDebug(BLUEZQT) << "Rfkill global state changed:" << d->m_state;
#endif
}

bool Rfkill::setSoftBlock(quint8 soft)
{
#ifndef Q_OS_LINUX
    Q_UNUSED(soft)
    return false;
#else
    if (!openForWriting()) {
        return false;
    }

    rfkill_event event;
    ::memset(&event, 0, sizeof(event));
    event.op = RFKILL_OP_CHANGE_ALL;
    event.type = RFKILL_TYPE_BLUETOOTH;
    event.soft = soft;

    bool ret = ::write(d->m_writeFd, &event, sizeof(event)) == sizeof(event);
    qCDebug(BLUEZQT) << "Setting Rfkill soft block succeeded:" << ret;
    return ret;
#endif
}

} // namespace BluezQt

#include "moc_rfkill.cpp"
