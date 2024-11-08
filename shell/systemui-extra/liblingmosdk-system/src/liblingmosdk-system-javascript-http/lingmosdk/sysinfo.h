/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#include <QWidget>
#include <QJsonArray>
#include <QJsonObject>
#include "lingmosdkdbus.h"

class Sysinfo : public QWidget
{
    Q_OBJECT
public:
    explicit Sysinfo(QWidget *parent = nullptr);
    Sysinfo(QString urlPath);
    int dbusConnect;
    Kysdkdbus kdbus;
    QString ServerName ;
    QString ServerInterface;
    QString ServerPath;
    QJsonObject deal_sysinfo();
    QJsonObject deal_sysinfo_path();
    QJsonObject deal_sysinfo_path_para();
private:
    QStringList path_list;
    QString path;

public slots:
    QString getSystemArchitecture();
    QString getSystemName();
    QString getSystemVersion(bool version);
    int getSystemActivationStatus();
    QString getSystemSerialNumber();
    QString getSystemKernelVersion();
    QString getSystemEffectUser();
    QString getSystemProjectName();
    QString getSystemProjectSubName();
    int getSystemProductFeatures();
    QString getSystemHostVirtType();
    QString getSystemHostCloudPlatform();
    QString getSystemOSVersion();
    QString getSystemUpdateVersion();
    bool getSystemIsZYJ();
    QStringList getSysLegalResolution();

signals:

};

#endif // SYSINFO_H
