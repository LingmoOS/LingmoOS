// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MONITORFULLWIDGET_H
#define MONITORFULLWIDGET_H

#include <DPlatformWindowHandle>
#include <DWindowManagerHelper>
#include <QFrame>

DWIDGET_USE_NAMESPACE

class MonitorIndicator : public QFrame
{
    Q_OBJECT

public:
    explicit MonitorIndicator(QWidget *parent = 0);

protected:
    void resizeEvent(QResizeEvent *e);
};

#endif // MONITORFULLWIDGET_H
