// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef STARTTOOLTIP_H
#define STARTTOOLTIP_H

#include <DWidget>
#include <dwindowmanager.h>

DWM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class StartTooltip : public DWidget
{
    Q_OBJECT

public:
    explicit StartTooltip(DWidget *parent = 0);
    void setWindowManager(DWindowManager *wm);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *, QEvent *event);

private:
    QString text;

    QPixmap iconImg;
    DWindowManager *windowManager;
};

#endif
