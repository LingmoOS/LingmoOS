// Copyright (C) 2023 ~ 2023 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "netcheckmodel.h"
#include "../defsecuritytoolsmnd.h"

#include <DFrame>
#include <DLineEdit>
#include <DSuggestButton>

#include <QPushButton>
#include <QMenu>
#include <QLabel>
#include <QTimer>

class NetCheckItem;

DWIDGET_USE_NAMESPACE
class NetCheckWidget : public DFrame
{
    Q_OBJECT
public:
    explicit NetCheckWidget(NetCheckModel *model, QWidget *parent = nullptr);
    ~NetCheckWidget();

private:
    // 初始化
    void initUI();
    void initData();

    // 加载网络设置界面
    QWidget *loadNetConnSettingUI();
    // 加载网络检测界面
    QWidget *loadNetConnCheckUI();
    // 检测开始/完成控件显示
    void showCheckWidget(bool status);

public Q_SLOTS:
    void selectNetType(QAction *action);
    void checkNetConnect();

    // 更新检测时间显示
    void updateTime();

    // 重置检测项
    void resetNetCheckItem();
    void acceptNetDeviceCheck(int result);
    void acceptNetSettingCheck(int result);
    void acceptNetDHCPCheck(int result);
    void acceptNetDNSCheck(int result);
    void acceptNetHostCheck(int result);
    void acceptNetConnCheck(int result);

    // 取消检测
    void cancelNetCheck();
    // 重新检测
    void restartNetCheck();
    // 完成
    void finishNetCheck();

private:
    NetCheckModel *m_model;

    QWidget *m_netSettingwidget;
    QWidget *m_netCheckwidget;

    QPushButton *m_netSettingBtn;
    QMenu *m_netSettingMenu;
    QString m_lastNetType;

    QWidget *m_domainWidget;
    DLineEdit *m_domainEdit;
    DSuggestButton *m_detectNowBtn; // 立即检测

    QString m_domainText;
    QTimer *m_timer; // 定时器
    int m_countTime;
    QLabel *m_netCheckInfo;
    QLabel *m_timeLabel;
    NetCheckItem *m_netDeviceFrame;
    NetCheckItem *m_netSettingFrame;
    NetCheckItem *m_netDHCPFrame;
    NetCheckItem *m_netDNSFrame;
    NetCheckItem *m_netHostFrame;
    NetCheckItem *m_netConnFrame;
    QPushButton *m_cancelBtn; // 取消检测
    QPushButton *m_restartBtn; // 重新检测
    DSuggestButton *m_finishBtn; // 重新检测

    bool m_checkingFlag;
    bool m_checkInfoErrorFlag;
};

class NetCheckMainWindow : public DefSecurityToolsMnd
{
    Q_OBJECT
public:
    explicit NetCheckMainWindow(QWidget *pParent = nullptr);
    ~NetCheckMainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

Q_SIGNALS:
    void sendWindowClose();

private:
    NetCheckWidget *m_pNetCheckWidget;
};
