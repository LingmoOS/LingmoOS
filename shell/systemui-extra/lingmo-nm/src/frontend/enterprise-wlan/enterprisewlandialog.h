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
#ifndef ENTERPRISEWLANDIALOG_H
#define ENTERPRISEWLANDIALOG_H
#include <QDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QPainter>

#include "securitypage.h"
#include "divider.h"
#include "kywirelessnetitem.h"
#include "coninfo.h"

class EnterpriseWlanDialog : public QWidget
{
    Q_OBJECT
public:
    EnterpriseWlanDialog(KyWirelessNetItem &wirelessNetItem, QString device, QWidget *parent = nullptr);
    ~EnterpriseWlanDialog();

protected:
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void initUI(); //初始化UI界面
    void centerToScreen();
    void initConnections();
    void initData();

private:
    KyWirelessNetItem m_wirelessNetItem;
    ConInfo m_info;
    QString m_deviceName;
    KyWirelessConnectOperation *m_connectOperation = nullptr;
    KyWirelessNetResource *m_resource = nullptr;

    /* 弹窗布局
     * Connect Enterprise WLAN············X
     * SSID··························[SSID]
     * -----------SecurityPage-------------
     * |                                  |
     * ------------------------------------
     * ····················CANCEL···CONNECT
     */
    QVBoxLayout  *m_mainLayout = nullptr;

    QHBoxLayout  *m_titleLayout = nullptr;
    QLabel       *m_titleLabel = nullptr;
    QPushButton  *m_closeBtn = nullptr;

    QWidget      *m_centerWidget = nullptr;
    QLabel       *m_descriptionLabel = nullptr;
    QLabel       *m_ssidLabel = nullptr;

    SecurityPage *m_securityPage = nullptr;

    QScrollArea *m_enterWlanScrollArea = nullptr;

    Divider *m_bottomDivider = nullptr;

    QPushButton  *m_cancelBtn = nullptr;
    QPushButton  *m_connectBtn = nullptr;

private Q_SLOTS:
    void onBtnConnectClicked();
    void onEapTypeChanged(const KyEapMethodType &type);

Q_SIGNALS:
    void enterpriseWlanDialogClose(bool);
};

#endif // ENTERPRISEWLANDIALOG_H
