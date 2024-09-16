// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUS_ADAPTOR
#define DBUS_ADAPTOR

#include "mainwindow.h"

#include <QtDBus>

/**
* @brief CloseDialog　窗口大小适配
* 窗口最小化后双击桌面图标弹出相应大小的窗口
*/
class ApplicationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT

public:
    explicit ApplicationAdaptor(CMainWindow *mw = nullptr);
    Q_CLASSINFO("D-Bus Interface", "com.deepin.camera")

    ~ApplicationAdaptor();

public slots:
    /**
    * @brief Raise 升起窗口信号
    */
    void Raise();

private:
    CMainWindow *m_mw;
};


#endif /* ifndef DBUS_ADAPTOR */

