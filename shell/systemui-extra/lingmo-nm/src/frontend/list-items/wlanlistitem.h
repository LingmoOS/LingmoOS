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
#ifndef WLANLISTITEM_H
#define WLANLISTITEM_H
#include "listitem.h"
#include "kywirelessnetitem.h"
#include "kywirelessnetresource.h"
#include "wlanpage.h"
#include "kywirelessconnectoperation.h"
#include <QCheckBox>
#include "lingmoactiveconnectresource.h"
#include <QAction>
#include "enterprisewlandialog.h"

#include <networkmanagerqt/wirelesssecuritysetting.h>

#include "kwidget.h"
#include "kpasswordedit.h"

using namespace kdk;

#define PSK_SETTING_NAME "802-11-wireless-security"

#define NORMAL_HEIGHT 48
#define EXPANDED_HEIGHT 120
#define PWD_LENGTH_LIMIT 8

#define EXCELLENT_SIGNAL 80
#define GOOD_SIGNAL 55
#define OK_SIGNAL 30
#define LOW_SIGNAL 5
#define NONE_SIGNAL 0

#define FREQ_5GHZ 5000

class WlanListItem : public ListItem
{
    Q_OBJECT
public:
    WlanListItem(KyWirelessNetItem &wirelessNetItem, QString device, bool isApMode, bool isShowWifi6Plus, QWidget *parent = nullptr);
    WlanListItem(KyWirelessNetItem &wirelessNetItem, QString device, bool isShowWifi6Plus, QWidget *parent = nullptr);
    WlanListItem(QWidget *parent = nullptr);
    ~WlanListItem();

public:
    QString getSsid();

    QString getUuid();

    QString getPath();

    void setSignalStrength(const int &signal);
    int  getSignalStrength();

    bool isConfigured();

    void setWlanState(const int &state);
    void setExpanded(const bool &expanded);

    ConnectState getConnectionState();
    void updateConnectState(ConnectState state);

    void updateWirelessNetSecurity(QString ssid, QString securityType);
    void updateWirelessNetItem(KyWirelessNetItem &wirelessNetItem);

    void forgetPwd();

    void setFrequency();

protected:
    void resizeEvent(QResizeEvent *event);
    void onRightButtonClicked();
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void itemHeightChanged(const bool isExpanded, const QString &ssid);

private:
    void initWlanUI();
    void refreshIcon(bool isActivated);

private:
    KyWirelessNetItem m_wirelessNetItem;
    KyWirelessConnectOperation *m_wirelessConnectOperation = nullptr;
    EnterpriseWlanDialog *enterpriseWlanDialog = nullptr;
    KyNetworkDeviceResourse *m_deviceResource = nullptr;

    bool m_hasPwd = true;
    QString m_wlanDevice;

    bool isEnterpriseWlanDialogShow = false;

    //密码输入区域的UI
    QFrame *m_pwdFrame = nullptr;
    QHBoxLayout *m_pwdFrameLyt = nullptr;
    KPasswordEdit *m_pwdLineEdit = nullptr;
    FixPushButton *m_connectButton = nullptr;

    //自动连接选择区域UI
    QFrame *m_autoConnectFrame = nullptr;
    QHBoxLayout *m_autoConnectFrameLyt = nullptr;
    QCheckBox *m_autoConnectCheckBox = nullptr;
    QLabel *m_autoConnectLabel = nullptr;

    bool m_forgetConnection = false;
    bool m_isApMode = false;
    bool m_isShowWifi6Plus = true;

protected Q_SLOTS:
    void onInfoButtonClicked();

private Q_SLOTS:
    void onNetButtonClicked();
    void onPwdEditorTextChanged();
    void onConnectButtonClicked();
    void onMenuTriggered(QAction *action);
    void onEnterpriseWlanDialogClose(bool isShow);
};

#endif // WLANLISTITEM_H
