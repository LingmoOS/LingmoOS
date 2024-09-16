// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pinscreenshots.h"
#include "mainwindow.h"
#include <DWidgetUtil>
#include <QDebug>

PinScreenShots::PinScreenShots(QObject *parent) : QObject(parent)
{
    m_loadingCount = 0;
}

bool PinScreenShots::openFile(QString filePath)
{
    qDebug() << __FUNCTION__ << __LINE__ << filePath;
    bool bRet = false;
    MainWindow *win = new MainWindow();
    //增加判断，空图片不会启动
    bRet = win->openFile(filePath);
    if (bRet) {
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
    return bRet;
}

void PinScreenShots::openImage(QImage image)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        qDebug() << __FUNCTION__ << __LINE__ << image.size();
        MainWindow *win = new MainWindow();
        win->openImage(image);
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
}

void PinScreenShots::openImageAndName(QImage image, QString imageName, QPoint point)
{
    //增加判断，空图片不会启动
    if (!image.isNull() && image.width() >= 1) {
        MainWindow *win = new MainWindow();
        win->openImageAndName(image, imageName, point);
        // 防止双屏模式下，贴图尺寸被改变
        Dtk::Widget::moveToCenter(win);
        win->move(win->getShowPosition());
        win->show();
        //第一次启动才居中
        if (m_loadingCount == 0) {
            m_loadingCount++;
        }
    }
}
