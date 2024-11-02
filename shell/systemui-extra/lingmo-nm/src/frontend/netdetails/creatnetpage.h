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
#ifndef CREATNETPAGE_H
#define CREATNETPAGE_H

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

#include "coninfo.h"
#include "multiplednswidget.h"

class CreatNetPage : public QFrame
{
    Q_OBJECT
public:
    CreatNetPage(QWidget *parent = nullptr);

    void constructIpv4Info(KyConnectSetting &setting);
    void startLoading();
    void stopLoading();
    void showIpv4AddressConflict(bool isConflict);
private:
    LineEdit *connNameEdit;
    QComboBox *ipv4ConfigCombox;
    LineEdit *ipv4addressEdit;
    LineEdit *netMaskEdit;
    LineEdit *gateWayEdit;

    QFormLayout *m_detailLayout;
    QVBoxLayout *mvBoxLayout;
    QLabel *m_connNameLabel;
    QLabel *m_configLabel;
    QLabel *m_addressLabel;
    QLabel *m_maskLabel;
    QLabel *m_gateWayLabel;

    QLabel *m_addressHintLabel;
    QLabel *m_maskHintLabel;

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
    bool checkConnectBtnIsEnabled();

    void initConflictHintLable();
    void initLoadingIcon();

    bool netMaskIsValide(QString text);
    QString getNetMaskText(QString text);

private Q_SLOTS:
    void setEnableOfSaveBtn();
    void configChanged(int index);
    void onAddressTextChanged();
    void onNetMaskTextChanged();
    void onAddressEditFinished();

public Q_SLOTS:
    void updateIcon();

Q_SIGNALS:
    void setCreatePageState(bool);
    void ipv4EditFinished(QString);

};

#endif // CREATNETPAGE_H
