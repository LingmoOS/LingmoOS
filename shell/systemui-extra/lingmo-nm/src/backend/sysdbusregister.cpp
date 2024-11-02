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
#include "sysdbusregister.h"

#include <QDebug>
#include <QSettings>
#include <QSharedPointer>

SysdbusRegister::SysdbusRegister()
{
}

SysdbusRegister::~SysdbusRegister()
{
}

void SysdbusRegister::systemRun(QString cmd){
    QProcess::execute(cmd);
}

QStringList SysdbusRegister::getWifiInfo(QString wifiName) {
    //返回名为wifiName的企业wifi的配置信息，包括eap,inner和用户列表
    QStringList wlist;
    QString filename = "/usr/share/lingmo-nm/wpaconn.conf";
    QFile file(filename);
    if(!file.exists()) {
        return wlist;
    }
    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
    wlist << autoSettings.get()->value("eap").toString();
    wlist << autoSettings.get()->value("inner").toString();
    int size = autoSettings.get()->beginReadArray(wifiName);
    for (int i = 0; i < size; ++i) {
        autoSettings.get()->setArrayIndex(i);
        wlist << autoSettings.get()->value("user").toString();
    }
    autoSettings.get()->endArray();
    return wlist;
}

bool SysdbusRegister::appendWifiInfo(QString name, QString eap, QString inner, QString user) {
    //向配置文件添加名为name的wifi配置，包括eap,inner和它的第一个用户
    QString filename = "/usr/share/lingmo-nm/wpaconn.conf";
    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
    autoSettings.get()->beginGroup(name);
    autoSettings.get()->setValue("eap", eap);
    autoSettings.get()->setValue("inner", inner);
    //以数组形式写入用户名
    autoSettings.get()->beginWriteArray(name);
    autoSettings.get()->setArrayIndex(0);
    autoSettings.get()->setValue("user", user);
    autoSettings.get()->endArray();
    return true;
}

bool SysdbusRegister::appendWifiUser(QString name, QString user) {
    //向名为name的wifi用户列表添加名为user的用户名
    QString filename = "/usr/share/lingmo-nm/wpaconn.conf";
    QSharedPointer<QSettings>  autoSettings = QSharedPointer<QSettings>(new QSettings(filename, QSettings::IniFormat));
    autoSettings.get()->beginWriteArray(name);
    autoSettings.get()->setArrayIndex(0);
    //读到用户名列表长度并在队尾添加一个用户名
    int size = autoSettings.get()->beginReadArray(name);
    autoSettings.get()->setArrayIndex(size);
    autoSettings.get()->setValue("user", user);
    autoSettings.get()->endArray();
    return true;
}
