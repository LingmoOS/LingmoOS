/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "sysdbusregister.h"

#include <QDebug>
#include <QRegExp>
#include <cstdlib>

SysdbusRegister::SysdbusRegister() {
}

SysdbusRegister::~SysdbusRegister() {
}

void SysdbusRegister::exitService() {
    qApp->exit(0);
}

/*
 *
 *   sprintf(cmd, "echo 9999999 | sudo tee -a /proc/sys/fs/inotify/max_user_watches");
 *   sprintf(cmd, "sysctl -w fs.inotify.max_user_watches=\"9999999\"");
 *   sprintf(cmd, "echo fs.inotify.max_user_watches=9999999 | sudo tee -a /etc/sysctl.conf");
*/

QString SysdbusRegister::setInotifyMaxUserWatchesStep1() {
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "echo 9999999 | sudo tee -a /proc/sys/fs/inotify/max_user_watches");
    if((fp = popen(cmd, "r")) != NULL) {
        rewind(fp);
        while(!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    } else {
        return QString("popen open failed");
    }
    return QString(ba);
}

QString SysdbusRegister::setInotifyMaxUserWatchesStep2() {
    QFile file("/proc/sys/fs/inotify/max_user_watches");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream ts(&file);
    QString s = ts.read(512);
    file.close();
    if(s.toInt() >= 9999999 ) {
        return s;
    }

    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "sysctl -w fs.inotify.max_user_watches=\"9999999\"");
    if((fp = popen(cmd, "r")) != NULL) {
        rewind(fp);
        while(!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    } else {
        return QString("popen open failed");
    }
    return QString(ba);
}

QString SysdbusRegister::setInotifyMaxUserWatchesStep3() {
    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "echo fs.inotify.max_user_watches=9999999 | sudo tee -a /etc/sysctl.conf");
    if((fp = popen(cmd, "r")) != NULL) {
        rewind(fp);
        while(!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    } else {
        return QString("popen open failed");
    }
    return QString(ba);
}

int SysdbusRegister::AddInotifyMaxUserInstance(int addNum)
{
    QFile file("/proc/sys/fs/inotify/max_user_instances");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;
    QTextStream ts(&file);
    QString s = ts.read(512);
    file.close();
    int instances = s.toInt() + addNum;

    QByteArray ba;
    FILE * fp = NULL;
    char cmd[128];
    char buf[1024];
    sprintf(cmd, "sysctl -w fs.inotify.max_user_instances=\"%d\"", instances);
    if((fp = popen(cmd, "r")) != NULL) {
        rewind(fp);
        while(!feof(fp)) {
            fgets(buf, sizeof(buf), fp);
            ba.append(buf);
        }
        pclose(fp);
        fp = NULL;
    } else {
        qWarning() << "popen open failed";
        return -1;
    }
    return instances;

}

//The following example comes from control center

//void SysdbusRegister::setAutoLoginStatus(QString username) {
//    QString filename = "/etc/lightdm/lightdm.conf";
//    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
//    autoSettings->beginGroup("SeatDefaults");

//    autoSettings->setValue("autologin-user", username);

//    autoSettings->endGroup();
//    autoSettings->syncIndexFileTypes();
//}

//QString SysdbusRegister::getSuspendThenHibernate() {
//    mHibernateSet->beginGroup("Sleep");

//    QString time = mHibernateSet->value("HibernateDelaySec").toString();

//    mHibernateSet->endGroup();
//    mHibernateSet->syncIndexFileTypes();

//    return time;
//}

//void SysdbusRegister::setPasswdAging(int days, QString username) {
//    QString cmd;

//    cmd = QString("chage -M %1 %2").arg(days).arg(username);
//    QProcess::execute(cmd);
//}
