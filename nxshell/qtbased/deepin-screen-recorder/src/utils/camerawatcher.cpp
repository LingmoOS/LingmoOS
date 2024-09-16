// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "camerawatcher.h"

#include <QThread>
#include <QCameraInfo>
#include <QDebug>

CameraWatcher::CameraWatcher(QObject *parent)
    : QObject(parent)
    , m_coulduse(true)
{
    //m_isRecoding = false;
    m_watchTimer = new QTimer(this);
    connect(m_watchTimer, &QTimer::timeout, this, &CameraWatcher::slotCameraWatcher); //新增定时器检测摄像头
}

CameraWatcher::~CameraWatcher()
{

}

void CameraWatcher::setWatch(const bool isWatcher)
{
    if (isWatcher) {
        m_watchTimer->start(1000);
    } else {
        m_watchTimer->stop();
    }
}

// 设置Coulduse的值
void CameraWatcher::setCoulduseValue(bool value)
{
    m_coulduse = value;
}

// 将原有的run方法替换为slotCameraWatcher，解决截图录屏退出时缓慢的问题
void CameraWatcher::slotCameraWatcher()
{
    bool couldUse = false;
    //qDebug() << "QCameraInfo::availableCameras()" << QCameraInfo::defaultCamera().deviceName();
    if (QCameraInfo::availableCameras().count() > 0) {
        couldUse = true;
    }

    if (couldUse != m_coulduse) {
        //发送log信息到UI
        m_coulduse = couldUse;
        emit sigCameraState(couldUse);
    }
}

