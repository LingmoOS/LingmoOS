/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#ifndef LINGMOWIREDCONNECTOPERATION_H
#define LINGMOWIREDCONNECTOPERATION_H

#include <QSettings>
#include <QDir>

#include "lingmonetworkresourcemanager.h"
#include "lingmoconnectsetting.h"
#include "lingmoconnectoperation.h"

const QString WIRED_NETWORK_STATE_CONF_FILE = QDir::homePath() + "/.config/lingmo/lingmo-nm-wired.ini";

class KyWiredConnectOperation : public KyConnectOperation
{
    Q_OBJECT
public:
    explicit KyWiredConnectOperation(QObject *parent = nullptr);
    ~KyWiredConnectOperation();

public:
    //无线开关
    void setWiredEnabled(bool enabled);
    bool getWiredEnabled();

    void  createWiredConnect(KyConnectSetting &connectSettingsInfo);
    void  updateWiredConnect(const QString &connectUuid, const KyConnectSetting &connectSettingsInfo);
    void  deleteWiredConnect(const QString &connectUuid);
    void  activateWiredConnection(const QString connectUuid, const QString devName);
    void  deactivateWiredConnection(const QString activeConnectName, const QString &activeConnectUuid);

    int closeWiredNetworkWithDevice(QString deviceName);
    int openWiredNetworkWithDevice(QString deviceName);

Q_SIGNALS:
    void wiredEnabledChanged(bool);
private:
    void getActiveConnection(QString &deviceName, QString &connectUuid);
    void saveActiveConnection(QString &deviceName, QString &connectUuid);
};

#endif // LINGMOWIREDCONNECTOPERATION_H
