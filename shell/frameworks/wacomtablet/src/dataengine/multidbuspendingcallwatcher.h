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

#ifndef MULTIDBUSPENDINGCALLWATCHER_H
#define MULTIDBUSPENDINGCALLWATCHER_H

#include <QDBusPendingCallWatcher>
#include <QObject>

class MultiDBusPendingCallWatcher : public QObject
{
    Q_OBJECT
public:
    explicit MultiDBusPendingCallWatcher(const QList<QDBusPendingCall> &calls, QObject *parent = 0);

signals:
    void finished(const QList<QDBusPendingCallWatcher *> &watchers);

private:
    QList<QDBusPendingCallWatcher *> m_watchers;
    int m_unfinished;
};

#endif // MULTIDBUSPENDINGCALLWATCHER_H
