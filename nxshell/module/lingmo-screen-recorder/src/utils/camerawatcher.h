// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CAMERAWATCHER_H
#define CAMERAWATCHER_H

#include <QObject>
#include <QTimer>
#include <QMutex>

class CameraWatcher : public QObject
{
    Q_OBJECT
public:
    explicit CameraWatcher(QObject *parent = nullptr);
    ~CameraWatcher();
    void setWatch(const bool isWatcher);
    // 设置Coulduse的值
    void setCoulduseValue(bool value);
public slots:
    // 将原来的run()方法改为定时器的槽函数，便于截图快速退出
    // 取消之前的线程方式，采用定时器监测
    void slotCameraWatcher();

signals:
    void sigCameraState(bool couldUse);


private:
    bool m_coulduse;
    QTimer *m_watchTimer = nullptr; //新增摄像头监视
};

#endif // CAMERAWATCHER_H

