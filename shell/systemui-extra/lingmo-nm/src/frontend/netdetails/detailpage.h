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
#ifndef DETAILPAGE_H
#define DETAILPAGE_H
#include <QWidget>
#include <QLabel>
#include <QFormLayout>
#include <QPainter>
#include <QCheckBox>
#include <QSpacerItem>
#include <QDebug>
#include <QListWidget>
#include <QApplication>
#include <QPushButton>
#include <QClipboard>

#include "detailwidget.h"
#include "coninfo.h"
#include "kwidget.h"
#include "ktabbar.h"
#include "kballontip.h"
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

using namespace kdk;

class DetailPage : public QFrame
{
    Q_OBJECT
public:
    DetailPage(bool isWlan, bool isCreate = false, QWidget *parent = nullptr);
    ~DetailPage();
    void setSSID(const QString &ssid);
    void setProtocol(const QString &protocol);
    void setSecType(const QString &secType);
    void setHz(const QString &hz);
    void setChan(const QString &chan);
    void setBandWidth(const QString &brandWidth);
    void setIpv4(const QString &ipv4);
    void setIpv4Dns(const QString &ipv4Dns);
    void setIpv6(const QString &ipv6);
    void setMac(const QString &mac);
    void setAutoConnect(bool flag);

    bool checkIsChanged(const ConInfo info);

    void getSsid(QString &ssid);

private:
    void initUI();
    void addDetailItem(QListWidget *listWidget, QWidget *detailWidget);
    void newCopiedTip();
    QPalette getTheme();
    void setInteractionFlag();

public:
    QListWidget  *m_listWidget = nullptr;
    DetailWidget *m_ssidWidget = nullptr;
    DetailWidget *m_protocolWidget = nullptr;
    DetailWidget *m_secTypeWidget = nullptr;
    DetailWidget *m_hzWidget = nullptr;
    DetailWidget *m_chanelWidget = nullptr;
    DetailWidget *m_bandwidthWidget = nullptr;
    DetailWidget *m_ipv4Widget = nullptr;
    DetailWidget *m_ipv4DnsWidget = nullptr;
    DetailWidget *m_ipv6Widget = nullptr;
    DetailWidget *m_macWidget = nullptr;

    QPushButton  *m_netCopyButton = nullptr;
    LineEdit     *m_SSIDEdit = nullptr;
    QLabel       *m_SSIDLabel = nullptr;
    QLabel       *m_Protocol = nullptr;
    QLabel       *m_SecType = nullptr;
    QLabel       *m_Hz = nullptr;
    QLabel       *m_Chan = nullptr;
    QLabel       *m_BandWidth = nullptr;
    QLabel       *m_IPV4 = nullptr;
    FixLabel     *m_IPV4Dns = nullptr;
    FixLabel     *m_IPV6 = nullptr;
    QLabel       *m_Mac = nullptr;
    QLabel       *m_autoConnect = nullptr;
    KBallonTip   *m_copiedTip = nullptr;

//    QWidget *autoFrame;
private:

    QVBoxLayout  *m_layout = nullptr;
    QVBoxLayout  *m_DetailLayout = nullptr;
    QHBoxLayout  *m_AutoLayout = nullptr;
    QCheckBox    *m_forgetNetBox = nullptr;
    bool         m_IsWlan = false;
    bool         m_IsCreate = false;

    QWidget *m_autoConWidget = nullptr;

    QString      m_formerSSID = nullptr;
    QString      m_formerIPV6 = nullptr;

private Q_SLOTS:
    void setEnableOfSaveBtn();
    void on_btnCopyNetDetail_clicked();

Q_SIGNALS:
     void setDetailPageState(bool);

};

#endif // DETAILPAGE_H
