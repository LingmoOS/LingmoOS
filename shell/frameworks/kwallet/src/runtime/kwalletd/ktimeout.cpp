/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ktimeout.h"
#include <QTimerEvent>

KTimeout::KTimeout(QObject *parent)
    : QObject(parent)
{
}

KTimeout::~KTimeout()
{
}

void KTimeout::clear()
{
    for (int timerId : std::as_const(_timers)) {
        killTimer(timerId);
    }
    _timers.clear();
}

void KTimeout::removeTimer(int id)
{
    const int timerId = _timers.value(id, 0);
    if (timerId != 0) {
        killTimer(timerId);
    }
    _timers.remove(id);
}

void KTimeout::addTimer(int id, int timeout)
{
    if (_timers.contains(id)) {
        return;
    }
    _timers.insert(id, startTimer(timeout));
}

void KTimeout::resetTimer(int id, int timeout)
{
    int timerId = _timers.value(id, 0);
    if (timerId != 0) {
        killTimer(timerId);
        _timers.insert(id, startTimer(timeout));
    }
}

void KTimeout::timerEvent(QTimerEvent *ev)
{
    QHash<int, int>::const_iterator it = _timers.constBegin();
    for (; it != _timers.constEnd(); ++it) {
        if (it.value() == ev->timerId()) {
            Q_EMIT timedOut(it.key());
            return;
        }
    }
}

#include "moc_ktimeout.cpp"
