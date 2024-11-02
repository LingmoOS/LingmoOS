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
#ifndef MOBILEHOTSPOTWIDGET_H
#define MOBILEHOTSPOTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>

#include <QTranslator>
#include <QApplication>
#include <QTimer>

#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusMetaType>
#include <QScrollArea>

#include "titlelabel.h"
#include "kwidget.h"
#include "kswitchbutton.h"
#include "kpasswordedit.h"
#include "connectdevlistitem.h"
#include "blacklistpage.h"
#include "connectdevpage.h"
#include "fixlabel.h"
#include "ukcccommon.h"

using namespace kdk;
using namespace ukcc;

class MobileHotspotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MobileHotspotWidget(QWidget *parent = nullptr);
    ~MobileHotspotWidget();

private:
    QFrame *m_hotspotFrame = nullptr;
    QFrame *m_switchFrame = nullptr;          //开关
    QFrame *m_ApNameFrame = nullptr;          //wifi名称
    QFrame *m_passwordFrame = nullptr;        //密码
    QFrame *m_freqBandFrame = nullptr;        //频带
    QFrame *m_interfaceFrame = nullptr;       //网卡

    KSwitchButton *m_switchBtn;

    TitleLabel *m_hotspotTitleLabel;
    QLabel *m_switchLabel;
    QLabel *m_apNameLabel;
    FixLabel *m_pwdLabel;
    QLabel *m_pwdHintLabel;
    FixLabel *m_freqBandLabel;
    FixLabel *m_interfaceLabel;

    QFrame *switchAndApNameLine;
    QFrame *apNameAndPwdLine;
    QFrame *pwdAndfreqBandLine;
    QFrame *freqBandAndInterfaceLine;

    QVBoxLayout *m_Vlayout;

    QLineEdit *m_apNameLine;
    KPasswordEdit *m_pwdNameLine;

    QComboBox *m_freqBandComboBox;
    QComboBox *m_interfaceComboBox;

    FixLabel *m_interfaceWarnLabel;
    QWidget* m_warnWidget;

    QDBusInterface  *m_interface = nullptr;

    QString m_interfaceName = "";

    QGSettings *m_switchGsettings = nullptr;

    QString m_uuid = "";
    QString m_hostName = "";


    QLabel *m_statusLabel;
    QTimer *m_waitTimer = nullptr;
    QList<QIcon> m_loadIcons;
    int m_currentIconIndex=0;
    void updateLoadingIcon();

    void resetFrameSize();
    void initUI();
    void initDbusConnect();
    void initInterfaceInfo();
    void getApInfo();
    bool getApInfoBySsid(QString devName, QString ssid, QStringList &info);
    void setSwitchFrame();
    void setApNameFrame();
    void setPasswordFrame();
    void setFreqBandFrame();
    void setInterFaceFrame();

    void setUiEnabled(bool enable);
    void setWidgetHidden(bool isHidden);

    void showDesktopNotify(const QString &message);

    void updateBandCombox();

    QString getHostName();

    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event);

    QFrame* myLine();

    QDBusInterface  *m_activePathInterface = nullptr;
    QDBusInterface  *m_settingPathInterface = nullptr;

    ConnectdevPage * m_connectDevPage = nullptr;
    BlacklistPage *m_blacklistPage = nullptr;

    QString getActivePathByUuid();
    QString getSettingPathByUuid();
    void initNmDbus();
    void initActivePathInterface(QString path);
    void deleteActivePathInterface();
    void initSettingPathInterface(QString path);
    void deleteSettingPathInterface();
    void initConnectDevPage();
    void initBlackListPage();

    bool m_isUserSelect = true;  //是否用户操作

signals:

private slots:
    void onActivateFailed(QString errorMessage);
    void onDeactivateFailed(QString errorMessage);
    //设备插拔
    void onDeviceStatusChanged();
    void onDeviceNameChanged(QString oldName, QString newName, int type);
    //热点断开
    void onHotspotDeactivated(QString devName, QString ssid);
    //热点连接
    void onHotspotActivated(QString devName, QString ssid, QString uuid, QString activePath, QString settingPath);

    void onWirelessBtnChanged(bool state);

    void onActiveConnectionChanged(QString deviceName, QString ssid, QString uuid, int status);

    void onApLineEditTextEdit(QString text);

    void onPwdTextChanged();

    void onInterfaceChanged();

    void startLoading();
    void stopLoading();
};

#endif // MOBILEHOTSPOTWIDGET_H
