/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
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
#ifndef LINGMOMEDIASETHEADSETWIDGET_H
#define LINGMOMEDIASETHEADSETWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QPushButton>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <QGSettings>
#include <alsa/asoundlib.h>
#include "customstyle.h"
#include <ukcc/interface/ukcccommon.h>
#include "windowmanager/windowmanager.h"
#include "lingmostylehelper/lingmostylehelper.h"

//设置声卡的key controls
#define LEVEL_BASIC (1<<0)
#define LEVEL_ID (1<<2)
#define HW_CARD "hw:0"

//任务栏多屏显示声音应用获取屏幕可用区域接口
#define PANEL_SETTINGS      "org.lingmo.panel.settings"
#define PANEL_SIZE_KEY      "panelsize"
#define PANEL_POSITION_KEY  "panelposition"

class UkmediaHeadsetButtonWidget : public QWidget
{
    Q_OBJECT
public:
   UkmediaHeadsetButtonWidget(QString icon,QString text);
   ~UkmediaHeadsetButtonWidget();

private:
   QLabel *iconLabel;
   QLabel *textLabel;
};

class LingmoUIMediaSetHeadsetWidget : public QWidget
{
    Q_OBJECT

    enum PanelPosition{
        Bottom = 0, //!< The bottom side of the screen.
        Top,    //!< The top side of the screen.
        Left,   //!< The left side of the screen.
        Right   //!< The right side of the screen.
    };

public:
    LingmoUIMediaSetHeadsetWidget(QWidget *parent = nullptr);
    ~LingmoUIMediaSetHeadsetWidget();
    void initSetHeadsetWidget();
    int cset(char * name, char *card, char *c, int roflag, int keep_handle);
    void showControlId(snd_ctl_elem_id_t *id);
    int showControl(const char *space, snd_hctl_elem_t *elem,int level);
    void showWindow();
    void initPanelGSettings();
    QRect caculatePosition(); //任务栏多屏显示声音应用获取屏幕可用区域接口
    friend class DeviceSwitchWidget;
    friend class UkmediaMainWidget;
private:
    QToolButton *headphoneIconButton;
    QToolButton *headsetIconButton;
    QToolButton *microphoneIconButton;
    QLabel *headphoneLabel;
    QLabel *headsetLabel;
    QLabel *microphoneLabel;

    QLabel *selectSoundDeviceLabel;
    QPushButton *soundSettingButton;
    QPushButton *cancelButton;

    bool isShow = false;
    QGSettings *m_panelGSettings = nullptr;
    int m_panelPosition;
    int m_panelSize;

private Q_SLOTS:
    void headphoneButtonClickedSlot(); //点击headphone按钮
    void headsetButtonClickedSlot(); //点击headset 按钮
    void microphoneButtonClickedSlot(); //点击microphone 按钮
    void soundSettingButtonClickedSlot(); //点击声音设置按钮
    void cancelButtonClickedSlot(); //点击取消按钮
protected:
    void paintEvent(QPaintEvent *event);
};


#endif // LINGMOMEDIASETHEADSETWIDGET_H
