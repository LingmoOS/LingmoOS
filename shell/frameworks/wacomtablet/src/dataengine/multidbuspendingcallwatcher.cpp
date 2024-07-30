/*
 * Copyright (C) 2015 Weng Xuetian <wengxt@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "multidbuspendingcallwatcher.h"

MultiDBusPendingCallWatcher::MultiDBusPendingCallWatcher(const QList<QDBusPendingCall> &calls, QObject *parent)
    : QObject(parent)
    , m_unfinished(0)
{
    Q_FOREACH (const QDBusPendingCall &call, calls) {
        m_watchers << new QDBusPendingCallWatcher(call, this);
        if (!m_watchers.last()->isFinished()) {
            m_unfinished++;
        }

        connect(m_watchers.last(), &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
            Q_UNUSED(watcher);
            m_unfinished--;
            if (m_unfinished == 0) {
                emit finished(m_watchers);
            }
        });
    }
}

#include "moc_multidbuspendingcallwatcher.cpp"
