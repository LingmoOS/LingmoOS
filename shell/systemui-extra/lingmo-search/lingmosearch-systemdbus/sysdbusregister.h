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
#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <QObject>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QSettings>

class SysdbusRegister : public QObject {
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.lingmo.search.interface")

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

//private:
//    QString mHibernateFile;

//    QSettings *mHibernateSet;

//signals:
//    Q_SCRIPTABLE void nameChanged(QString);
//    Q_SCRIPTABLE void computerinfo(QString);

public Q_SLOTS:

    Q_SCRIPTABLE void exitService();
//    Q_SCRIPTABLE QString GetComputerInfo();

    Q_SCRIPTABLE QString setInotifyMaxUserWatchesStep1();
    Q_SCRIPTABLE QString setInotifyMaxUserWatchesStep2();
    Q_SCRIPTABLE QString setInotifyMaxUserWatchesStep3();
    Q_SCRIPTABLE int AddInotifyMaxUserInstance(int addNum);

//    // 设置免密登录状态
//    Q_SCRIPTABLE void setNoPwdLoginStatus();

//    // 获取免密登录状态
//    Q_SCRIPTABLE QString getNoPwdLoginStatus();

//    // 设置自动登录状态
//    Q_SCRIPTABLE void setAutoLoginStatus(QString username);

//    // 获取挂起到休眠时间
//    Q_SCRIPTABLE QString getSuspendThenHibernate();

//    // 设置挂起到休眠时间
//    Q_SCRIPTABLE void setSuspendThenHibernate(QString time);

//    // 设置密码时效
//    void setPasswdAging(int days, QString username);
};

#endif // SYSDBUSREGISTER_H
