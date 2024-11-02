/*
 * Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef SYSDBUSREGISTER_H
#define SYSDBUSREGISTER_H

#include <QObject>
#include <QProcess>
#include <QDBusError>
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusInterface>
#include <QString>

#include <QFile>

class SysdbusRegister : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.lingmo.NetworkManager.interface")

public:
    explicit SysdbusRegister();
    ~SysdbusRegister();

private:
//    QString m_name;

Q_SIGNALS:
//    Q_SCRIPTABLE void nameChanged(QString);
//    Q_SCRIPTABLE void computerinfo(QString);

public Q_SLOTS:

    Q_SCRIPTABLE void systemRun(QString cmd);
    Q_SCRIPTABLE QStringList getWifiInfo(QString wifiName);
    Q_SCRIPTABLE bool appendWifiInfo(QString name, QString eap, QString inner, QString user);
    Q_SCRIPTABLE bool appendWifiUser(QString name, QString user);

};

#endif // SYSDBUSREGISTER_H
