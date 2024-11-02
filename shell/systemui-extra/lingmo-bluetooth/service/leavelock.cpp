/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#include "leavelock.h"
#include "sessiondbusregister.h"

LeaveLock::LeaveLock(QObject *parent) : QObject(parent)
{
}

LeaveLock::~LeaveLock() {
}

bool LeaveLock::setPing(QString uuid, QString dev, bool on) {
    if (dev == "" || !on) {
        process->deleteLater();
        manualCancel = true;
        return true;
    }
    process = new QProcess();
    connect(process, &QProcess::readyReadStandardOutput, this, &LeaveLock::readOutput);
    connect(process, &QProcess::readChannelFinished, this, &LeaveLock::pingFinished);
    manualCancel = false;
    uid = uuid;
    lockdev = dev;
    qDebug() << "================" << lockdev;
    QString cmd = "l2ping " + lockdev;
    process->start(cmd);
    return true;
}

void LeaveLock::readOutput() {
    Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(process->readAllStandardOutput());
}

void LeaveLock::pingFinished() {
    QByteArray errorMsg = process->readAllStandardError();
    QByteArray output = process->readAllStandardOutput();
    qDebug() << "<<<<<<<<<<<<<<" << output << errorMsg;
    if (!output.isEmpty()) {
        Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(output);
    }
    if (!errorMsg.isEmpty()) {
        Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(errorMsg);
        return;
    }
    if (manualCancel)
        Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(QString("ping out").toLatin1());
    else
        Q_EMIT SYSDBUSMNG::getInstance()->pingTimeSignal(QString("loss connect").toLatin1());
    manualCancel = false;
}
