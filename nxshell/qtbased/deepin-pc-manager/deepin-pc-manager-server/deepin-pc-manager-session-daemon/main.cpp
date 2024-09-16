// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "operatedbusdata.h"
#include "window/modules/securitytooldialog/securitytooldialoginter.h"

#include <DApplication>
#include <DLog>
#include <DMainWindow>
#include <DWidgetUtil>

#include <QPushButton>

#include <unistd.h>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication a(argc, argv);
    a.loadTranslator();
    a.setAttribute(Qt::AA_EnableHighDpiScaling);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setOrganizationName("deepin");
    a.setApplicationName("deepin-pc-manager-session-daemon");
    a.setApplicationDisplayName(""); // 标题文字设置为空
    a.setQuitOnLastWindowClosed(false);

    // 设置异步线程退出时间，默认为30秒
    QThreadPool::globalInstance()->setExpiryTimeout(100);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    OperateDBusData dbus;
    SecurityToolDialogInter toolDialogDbusInter;

    return a.exec();
}
