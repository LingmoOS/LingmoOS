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
#ifndef IPV4PAGE_H
#define IPV4PAGE_H

#include <QWidget>
#include <QLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPainter>
#include <QScrollArea>
#include <QSpacerItem>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>

//#include "lingmoconnectsetting.h"
#include "coninfo.h"
#include "multiplednswidget.h"
#include "divider.h"

class Ipv4Page : public QFrame
{
    Q_OBJECT
public:
    Ipv4Page(QWidget *parent = nullptr);
    void setIpv4Config(KyIpConfigType ipv4Config);
    void setIpv4(const QString &ipv4);
    void setNetMask(const QString &netMask);
    void setMulDns(const QList<QHostAddress> &dns);
    void setGateWay(const QString &gateWay);
    void setUuid(QString uuid) {
        if (m_dnsWidget != nullptr) {
            m_dnsWidget->setUuid(uuid);
        }
    }

    QString getNetMaskText(QString text);

    bool checkIsChanged(const ConInfo info, KyConnectSetting &setting);
    bool checkDnsSettingsIsChanged();

    void startLoading();
    void stopLoading();
    void showIpv4AddressConflict(bool isConflict);
private:
    QComboBox *ipv4ConfigCombox;
    LineEdit *ipv4addressEdit;
    LineEdit *netMaskEdit;
    LineEdit *gateWayEdit;

    QFormLayout *m_detailLayout;
    QVBoxLayout *mvBoxLayout;
    QLabel *m_configLabel;
    QLabel *m_addressLabel;
    QLabel *m_maskLabel;
    QLabel *m_gateWayLabel;

    QLabel *m_configEmptyLabel;
    QLabel *m_addressHintLabel;
    QLabel *m_maskHintLabel;
    QLabel *m_gateWayEmptyLabel;
    QLabel *m_dnsEmptyLabel;

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
    void setLineEnabled(bool check);
    void configSave();
    bool getTextEditState(QString text);
    bool netMaskIsValide(QString text);

    bool checkConnectBtnIsEnabled();
    void initConflictHintLable();
    void initLoadingIcon();

private Q_SLOTS:
    void setEnableOfSaveBtn();
    void configChanged(int index);
    void onAddressTextChanged();
    void onNetMaskTextChanged();
    void onAddressEditFinished();
    void updateIcon();

Q_SIGNALS:
    void setIpv4PageState(bool);
    void ipv4EditFinished(const QString &address);
    void scrollToBottom();
};

#endif // IPV4PAGE_H
