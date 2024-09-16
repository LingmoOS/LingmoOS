// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SYSCHECKRESUTLWIDGET_H
#define SYSCHECKRESUTLWIDGET_H

#include "syscheckresultheaderwidget.h"
#include "syscheckresultitemwidget.h"

#include <DFrame>
#include <DPushButton>
#include <DSuggestButton>

#include <QList>
#include <QStandardItemModel>
#include <QWidget>

class SysCheckResultWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit SysCheckResultWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void onCheckFinished();
    void onFixing();
    void onFixed();
    void setResultModel(const QStandardItemModel &);
    void setHeaderContent(int point, int issueCount);
    void setFastFixValid(bool);
    void resetItemsBackGround();
    void setAutoStartAppCount(int);
    void onFixStarted(SysCheckItemID id);
    void onFixFinished(SysCheckItemID id);

Q_SIGNALS:
    void onFixItemStarted(SysCheckItemID id);
    void onFixItemFinished(SysCheckItemID id);
    void requestFixItem(SysCheckItemID id);
    void requestFixAll();

    void requestCheckAgain();
    void requestSetIgnore(SysCheckItemID, bool);

    void requestQuit();
    void autoStartAppCountChanged(int);

private:
    void initUI();
    void initConnection();
    void createItemWidgets();
    void hideAll();

private:
    SysCheckResultHeaderWidget *m_header;
    Dtk::Widget::DSuggestButton *m_checkedButton;
    Dtk::Widget::DSuggestButton *m_fixedButton;
    Dtk::Widget::DPushButton *m_recheckButton;
    QList<SysCheckResultItemWidget *> m_itemWidgets;
    QList<SysCheckItemID> m_currentFixingItems;
};

#endif // SYSCHECKRESUTLWIDGET_H
