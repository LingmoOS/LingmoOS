// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <DMenu>

#include <QMouseEvent>
#include <QDebug>
#include <QTimer>

DWIDGET_USE_NAMESPACE
class VNoteRightMenu : public DMenu
{
    Q_OBJECT
public:
    explicit VNoteRightMenu(QWidget *parent = nullptr);
    ~VNoteRightMenu() override;
    void setPressPoint(QPoint point);

protected:
    //初始化信号槽连接
    void initConnections();
    //处理鼠标move事件
    void mouseMoveEvent(QMouseEvent *eve) override;
    //处理鼠标release事件
    void mouseReleaseEvent(QMouseEvent *eve) override;

private:
    QPoint m_touchPoint;
    bool m_moved {false};
    QTimer *m_timer;
signals:
    //触摸移动信号
    void menuTouchMoved(bool isTouch);
    //触摸释放信号
    void menuTouchReleased();
private slots:
};
