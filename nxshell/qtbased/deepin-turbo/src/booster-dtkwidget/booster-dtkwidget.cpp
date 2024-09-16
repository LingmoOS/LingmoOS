/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (directui@nokia.com)
**
** This file is part of applauncherd
**
** If you have questions regarding the use of this file, please contact
** Nokia at directui@nokia.com.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/

#include "booster-dtkwidget.h"
#include "daemon.h"

#include <DApplication>
#include <DWindowManagerHelper>

#include <QWidget>
#include <QImageReader>

BEGIN_NAMESPACE
const string QWBooster::m_boosterType  = "dtkwidget";

const string & QWBooster::boosterType() const
{
    return m_boosterType;
}

void QWBooster::initialize(int initialArgc, char **initialArgv, int boosterLauncherSocket,
                           int socketFd, SingleInstance *singleInstance, bool bootMode)
{
    static int argc = initialArgc;
    // 默认开启高分辨率图标支持
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    Q_UNUSED(new DTK_WIDGET_NAMESPACE::DApplication(argc, initialArgv));

    Booster::initialize(initialArgc, initialArgv, boosterLauncherSocket, socketFd, singleInstance, bootMode);
}

bool QWBooster::preload()
{
    // 初始化QWidget，减少程序启动后第一次显示QWidget时的时间占用
    // 在龙芯和申威上，时间主要消耗在xcb插件中加载glx相关库（libdri600等）
    QWidget widget;

    widget.setWindowFlags(Qt::BypassWindowManagerHint
                          | Qt::WindowStaysOnBottomHint
                          | Qt::WindowTransparentForInput
                          | Qt::WindowDoesNotAcceptFocus);
    widget.setFixedSize(1, 1);
    widget.createWinId();
//    widget.show();

    // 当创建窗口时会初始化窗口的class name, 但是此时还未加载实际的程序, 缓存的class name
    // 无用, 因此要清理缓存的数据, 避免加载程序后的正常窗口的wm class name错误
    DTK_GUI_NAMESPACE::DWindowManagerHelper::setWmClassName(QByteArray());

    // 初始化图片解码插件，在龙芯和申威上，Qt程序冷加载图片解码插件几乎耗时1s
    Q_UNUSED(QImageReader::supportedImageFormats());

    return true;
}
END_NAMESPACE

int main(int argc, char **argv)
{
    // NOTE(lxz): booster will delete in daemon fork function
    DeepinTurbo::QWBooster* booster = new DeepinTurbo::QWBooster;

    DeepinTurbo::Daemon d(argc, argv);
    d.run(booster);
}
