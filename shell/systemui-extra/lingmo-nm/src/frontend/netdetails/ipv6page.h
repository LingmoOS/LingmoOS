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
#ifndef IPV6PAGE_H
#define IPV6PAGE_H

#include <QWidget>
#include <QLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPainter>
#include <QCheckBox>
#include <QScrollArea>
#include <QSpacerItem>
#include <QComboBox>
#include <QLineEdit>

//#include "lingmoconnectsetting.h"
#include "coninfo.h"
#include "multiplednswidget.h"

class Ipv6Page : public QFrame
{
    Q_OBJECT
public:
    Ipv6Page(QWidget *parent = nullptr);
    void setIpv6Config(KyIpConfigType ipv6Config);
    void setIpv6(const QString &ipv4);
    void setIpv6Perfix(const int &ipv6Perfix);
    void setMulDns(const QList<QHostAddress> &dns);
    void setGateWay(const QString &gateWay);

    bool checkIsChanged(const ConInfo info, KyConnectSetting &setting);

    int getPerfixLength(QString text);

    void startLoading();
    void stopLoading();
    void showIpv6AddressConflict(bool isConflict);

private:
    QComboBox *ipv6ConfigCombox;
    LineEdit *ipv6AddressEdit;
    LineEdit *lengthEdit;
    LineEdit *gateWayEdit;

    QFormLayout *m_detailLayout;
    QLabel *m_configLabel;
    QLabel *m_addressLabel;
    QLabel *m_subnetLabel;
    QLabel *m_gateWayLabel;

    QLabel *m_configEmptyLabel;
    QLabel *m_addressHintLabel;
    QLabel *m_subnetEmptyLabel;
    QLabel *m_gateWayHintLabel;

    MultipleDnsWidget *m_dnsWidget = nullptr;

    QLabel *m_statusLabel = nullptr;
    QList<QIcon> m_loadIcons;
    QTimer *m_iconTimer = nullptr;
    int m_currentIconIndex =0;

    QLabel *m_iconLabel;
    QLabel *m_textLabel;

private:
    void initUI();
    void initComponent();
    void setControlEnabled(bool check);

    bool getIpv6EditState(QString text);

    bool checkConnectBtnIsEnabled();

    void initConflictHintLable();
    void initLoadingIcon();

private Q_SLOTS:
    void configChanged(int index);
    void setEnableOfSaveBtn();
    void onAddressTextChanged();
    void onGatewayTextChanged();
    void onAddressEidtFinished();
    void updateIcon();

Q_SIGNALS:
    void setIpv6PageState(bool);
    void ipv6EditFinished(const QString &address);
    void scrollToBottom();
};

#endif // IPV6PAGE_H
