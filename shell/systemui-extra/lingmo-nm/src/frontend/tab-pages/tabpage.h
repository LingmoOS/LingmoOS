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
#ifndef TABPAGE_H
#define TABPAGE_H

#include "divider.h"
#include "kylable.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QListWidget>
#include <QDir>
#include <QComboBox>
#include <QEvent>
#include <QProcess>
#include <QDebug>
#include "lingmonetworkdeviceresource.h"
#include "kwidget.h"
#include "kswitchbutton.h"
//#include "kborderlessbutton.h"

#define EMPTY_SSID "emptyssid"

#define REFRESH_NETWORKSPEED_TIMER 1000

using namespace kdk;

#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define MAIN_LAYOUT_SPACING 0
#define TITLE_FRAME_HEIGHT 50     //TabWidget的tab和widget有间隙，和设计稿看起来一致就不能设为设计稿里的高度
#define TITLE_LAYOUT_MARGINS 24,0,24,0
#define DEVICE_LAYOUT_MARGINS 24,0,24,8
#define DEVICE_COMBOBOX_WIDTH 180
#define DEVICE_COMBOBOX_WIDTH_MAX 230
#define ACTIVE_NET_LAYOUT_MARGINS 8,8,8,8
#define NET_LAYOUT_MARGINS 8,8,0,1
#define NET_LAYOUT_SPACING 8
#define NET_LIST_SPACING 0
#define TEXT_MARGINS 16,0,0,0
#define TEXT_HEIGHT 20
//#define SCROLL_AREA_HEIGHT 200
#define SETTINGS_LAYOUT_MARGINS 23,0,24,0
#define TRANSPARENT_COLOR QColor(0,0,0,0)
#define INACTIVE_AREA_MIN_HEIGHT 170
#define ACTIVE_AREA_MAX_HEIGHT 92

#define MAX_ITEMS 4
#define MAX_WIDTH 412
#define MIN_WIDTH 404

#define SCROLL_STEP 4

enum KyDeviceType
{
    WIRED,
    WIRELESS
};

const QString CONFIG_FILE_PATH   =  QDir::homePath() + "/.config/lingmo/lingmo-nm.conf";
bool checkDeviceExist(KyDeviceType deviceType, QString deviceName);
QString getDefaultDeviceName(KyDeviceType deviceType);
void setDefaultDevice(KyDeviceType deviceType, QString deviceName);
void getDeviceEnableState(int type, QMap<QString, bool> &map);
bool getOldVersionWiredSwitchState(bool &state);

class TabPage : public QWidget
{
    Q_OBJECT
public:
    explicit TabPage(QWidget *parent = nullptr);
    ~TabPage();

//    void updateDefaultDevice(QString &deviceName);
//    QString getDefaultDevice();
    static void showDesktopNotify(const QString &message, QString soundName);

    void hideSetting() {
        if (nullptr != m_settingsFrame) {
            m_settingsFrame->hide();
            m_inactivatedNetDivider->hide();
            m_inactivatedNetFrame->setMinimumHeight(INACTIVE_AREA_MIN_HEIGHT + 100);
        }
    }
    void showSetting() {
        if (nullptr != m_settingsFrame) {
            m_inactivatedNetFrame->setMinimumHeight(INACTIVE_AREA_MIN_HEIGHT);
            m_settingsFrame->show();
            m_inactivatedNetDivider->show();
        }
    }

Q_SIGNALS:
    void deviceStatusChanged();
    void wirelessDeviceStatusChanged();
    void deviceNameChanged(QString oldName, QString newName, int type);
    void activateFailed(QString errorMessage);
    void deactivateFailed(QString errorMessage);

protected:
    void initUI();
    int getCurrentLoadRate(QString dev, long *save_rate, long *tx_rate);
//    virtual void initDevice() = 0;//初始化默认设备
    virtual void initDeviceCombox() = 0;//初始化设备选择下拉框
    QTimer *setNetSpeed = nullptr;
    QVBoxLayout * m_mainLayout = nullptr;
    QFrame * m_titleFrame = nullptr;
    QHBoxLayout * m_titleLayout = nullptr;
    QLabel * m_titleLabel = nullptr;
    KSwitchButton * m_netSwitch = nullptr;
    Divider * m_titleDivider = nullptr;

    QFrame * m_activatedNetFrame = nullptr;
    QVBoxLayout * m_activatedNetLayout = nullptr;
    QLabel * m_activatedNetLabel = nullptr;
    Divider * m_activatedNetDivider = nullptr;

    QFrame * m_inactivatedNetFrame = nullptr;
    QVBoxLayout * m_inactivatedNetLayout = nullptr;
    QLabel * m_inactivatedNetLabel = nullptr;
//    QScrollArea * m_inactivatedNetListArea = nullptr;
    QWidget * m_inactivatedNetListArea = nullptr;
    QVBoxLayout * m_inactivatedAreaLayout = nullptr;

    Divider * m_inactivatedNetDivider = nullptr;

    QFrame * m_settingsFrame = nullptr;
    QHBoxLayout * m_settingsLayout = nullptr;
    KyLable * m_settingsLabel = nullptr;
//    KBorderlessButton *m_settingsBtn = nullptr;

    //临时增加的下拉框选择网卡区域
    QFrame * m_deviceFrame = nullptr;
    QHBoxLayout * m_deviceLayout = nullptr;
    QLabel * m_deviceLabel = nullptr;
    QComboBox * m_deviceComboBox = nullptr;
    QLabel * m_tipsLabel = nullptr;

    long int start_rcv_rates = 0;	//保存开始时的流量计数
    long int end_rcv_rates = 0;	//保存结束时的流量计数
    long int start_tx_rates = 0;   //保存开始时的流量计数
    long int end_tx_rates = 0; //保存结束时的流量计数

public Q_SLOTS:
    virtual void onDeviceComboxIndexChanged(int currentIndex) = 0;
    void onPaletteChanged();

protected Q_SLOTS:
    void onSetNetSpeed(QListWidget* m_activatedNetListWidget, bool isActive, QString dev);
};

#endif // TABPAGE_H
