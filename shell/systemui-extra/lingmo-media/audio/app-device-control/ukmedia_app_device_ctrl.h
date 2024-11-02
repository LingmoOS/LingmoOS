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
#ifndef UKMEDIA_APP_DEVICE_CTRL_H
#define UKMEDIA_APP_DEVICE_CTRL_H

#include <QMap>
#include <glib.h>
#include <QModelIndex>
#include <QStandardItem>
#include <QStackedWidget>

#include <QDBusError>
#include <QDBusReply>
#include <QDBusVariant>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusConnection>
#include <pulse/pulseaudio.h>

#include "kwidget.h"
#include "knavigationbar.h"
#include "ukmedia_app_item_widget.h"
#include "../common/ukmedia_common.h"

using namespace kdk;

class UkmediaAppCtrlWidget : public KWidget
{
    Q_OBJECT

public:
    explicit UkmediaAppCtrlWidget(QWidget *parent = nullptr);
    ~UkmediaAppCtrlWidget();
    void initUI();
    void initData();
    void dealSlot();
    void addItem(QString appName);
    int getSystemVolume();
    bool getSystemMuteState();
    int getAppVolume(QString app);
    bool getAppMuteState(QString app);

    void getAppList();
    void getAllPortInfo();
    void getRecordAppInfo();
    void getPlaybackAppInfo();
    QStringList getAllInputPort();
    QStringList getAllOutputPort();
    QString getSystemInputDevice();
    QString getSystemOutputDevice();
    QString getAppInputDevice(QString app);
    QString getAppOutputDevice(QString app);

    int valueToPaVolume(int value);
    int paVolumeToValue(int value);

private:
    KNavigationBar* m_pAppSoundCtrlBar;
    QStackedWidget* stackWidget;

    QStringList appList;    //获取当前存在的应用名单
    QStringList sinkPortList;
    QStringList sourcePortList;
    QList<appInfo> recordAppInfoList;
    QList<appInfo> playbackAppInfoList;
    QMap<int, pa_device_port_info> portInfoMap;

    QDBusInterface* interface;

    bool setSystemVolume(int value);
    bool setSystemInputDevice(QString portLabel);
    bool setSystemOutputDevice(QString portLabel);
    int findAppDirection(QString appName);
#ifdef PA_PROP_APPLICATION_MOVE
    bool checkAppMoveStatus(int appType, QString appName);
#endif
    int indexOfItemCombobox(QString port, QComboBox *box);
    QString getAppName(QString appName);
    QString getAppIcon(QString appName);
    QString AppDesktopFileAdaption(QString appName);

private Q_SLOTS:
    void updatePort();
    void updateAppItem(QString appName);
    void removeAppItem(QString appName);
    void updateSystemVolume(int value);
    void updateSystemMuteState(bool state);
    void updateAppMuteState(QString app, bool state);
    void appVolumeChangedSlot(QString app, QString appId, int volume);
    bool setAppMuteState();
    bool setAppVolume(int value);
    bool setAppInputDevice(QString portLabel);
    bool setAppOutputDevice(QString portLabel);
};

#endif // UKMEDIA_APP_DEVICE_CTRL_H
