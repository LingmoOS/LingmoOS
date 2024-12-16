// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSCHECKREUSLTITEMWIDGET_H
#define SYSCHECKREUSLTITEMWIDGET_H

#include "systemcheckdefinition.h"

#include <DCommandLinkButton>
#include <DFrame>
#include <DLabel>
#include <DPushButton>
#include <DSpinner>
#include <DTipLabel>

#include <QStandardItemModel>
#include <QWidget>

class SysCheckResultItemWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit SysCheckResultItemWidget(SysCheckItemID id, QWidget *parent = nullptr);

    ~SysCheckResultItemWidget() { }

public Q_SLOTS:
    void onNotifyComplete(SysCheckItemID index, bool isAbnormal);
    void onNotifyFixStarted(SysCheckItemID index);
    void onNotifyFixFinished(SysCheckItemID index);
    void onNotifyIgnored(bool isIgnored);
    void setResult(const QStandardItemModel &);
    void onAutoStartAppCountChanged(int);

Q_SIGNALS:
    void requestFixItem(SysCheckItemID index);
    void requestSetIgnore(SysCheckItemID index, bool isNeedIgnore);
    void widgetHidden(); // 隐藏时需要刷新其它项的背景

private:
    void initUI();
    void initConnection();
    void hideAll();
    void setResult(QStandardItem *);

private:
    SysCheckItemID m_id;
    Dtk::Widget::DLabel *m_typeIconLabel;
    Dtk::Widget::DLabel *m_typeNameLabel;
    Dtk::Widget::DLabel *m_statusIconLabel;
    Dtk::Widget::DLabel *m_statusInfoLabel;
    Dtk::Widget::DTipLabel *m_fixTipLabel;
    // Dtk::Widget::DLabel *m_ignoreLabel;
    Dtk::Widget::DCommandLinkButton *m_jumpLabel;
    Dtk::Widget::DSpinner *m_fixSpinner;
    Dtk::Widget::DPushButton *m_ignoreBtn;
    Dtk::Widget::DPushButton *m_disIgnoreBtn;
    QStandardItem *m_resultItem;
};

#endif // SYSCHECKREUSLTITEMWIDGET_H
