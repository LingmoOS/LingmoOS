// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKPROCESSINGWIDGET_H
#define CHECKPROCESSINGWIDGET_H

#include "systemcheckdefinition.h"

#include <DFrame>
#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DTipLabel>

#include <QDateTime>
#include <QProgressBar>
#include <QTimer>
#include <QWidget>

class CheckProcessingWidget : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit CheckProcessingWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    // point为任务自身属性
    void onCheckMissionCompleted(int point);
    void onCheckMissionStarted(SysCheckItemID);
    void onCheckInitial();
    void onCheckDone();
    void ticktock();

Q_SIGNALS:
    void checkCanceled();
    void checkPrepared();
    void checkDone();

private:
    void initUI();
    void initConnection();

private:
    Dtk::Widget::DLabel *m_movieLable;
    Dtk::Widget::DLabel *m_stageLable;
    Dtk::Widget::DLabel *m_infoLable;
    Dtk::Widget::DProgressBar *m_progressBar;
    Dtk::Widget::DTipLabel *m_timeLable;
    Dtk::Widget::DPushButton *m_cancelButton;
    QTimer m_timer;
    QTime m_startTime;
    int m_progressValue;
};

#endif // CHECKPROCESSINGWIDGET_H
