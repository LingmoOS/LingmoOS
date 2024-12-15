// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gomokuapplication.h"
#include "gomokumainwindow.h"

Gomokuapplication::Gomokuapplication(int &argc, char **argv)
    : DApplication(argc, argv)
{

}

void Gomokuapplication::handleQuitAction()
{
    QWidget *pWgt = activeWindow();
    GomokuMainWindow *pWindow = qobject_cast<GomokuMainWindow *>(pWgt); //在对应的MainWindow操作

    // 处理点击退出事件
    if (pWindow)
        pWindow->handleQuit();
}
