/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Ding Jing <dingjing@kylinos.cn>
 *
 */

#ifndef SYNCTHREAD_H
#define SYNCTHREAD_H

#include <QObject>
#include <QProcess>
#include "explorer-core_global.h"
#include <libnotify/notify.h>

namespace Peony{

class PEONYCORESHARED_EXPORT SyncThread : public QObject
{
    Q_OBJECT
public:
    explicit SyncThread(QString uri, QObject *parent = nullptr);

    static void notifyUser(QString notifyContent);

Q_SIGNALS:
    void syncFinished();

public Q_SLOTS:
    void parentStartedSlot();

private:
    QString                         mHint;
    QString                         mUri;

};

}

#endif // SYNCTHREAD_H
