// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNMAINWNDDELAYINITTASK_H
#define VNMAINWNDDELAYINITTASK_H

#include "vntask.h"
#include "views/vnotemainwindow.h"

#include <QObject>

class VNoteMainWindow;
//延时任务线程，一些程序启动时不需要马上完成的任务，可以放在此线程中处理
class VNMainWndDelayInitTask : public VNTask
{
    Q_OBJECT
public:
    explicit VNMainWndDelayInitTask(VNoteMainWindow *pMainWnd, QObject *parent = nullptr);

signals:

public slots:

protected:
    //执行任务
    virtual void run();

protected:
    VNoteMainWindow *m_pMainWnd {nullptr};
};

#endif // VNMAINWNDDELAYINITTASK_H
