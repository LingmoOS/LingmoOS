// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSFORSYSTEMOMONITORPLUGINSERVCE_H
#define DBUSFORSYSTEMOMONITORPLUGINSERVCE_H

#include <QObject>
#include <QDBusContext>
class DBusForSystemoMonitorPluginServce : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.SystemMonitorMain")
public:
    DBusForSystemoMonitorPluginServce(QObject *parent = nullptr);

public slots:
    //!
    //! \brief slotJumpDetailWidget 通过注册的DBUS接口，获取当前消息CODE，判断需要跳转到详情界面的画面
    //! \param msgCode 消息码
    //!
    Q_SCRIPTABLE void slotJumpDetailWidget(const QString &msgCode);

    //!
    //! \brief slotJumpProcessWidget 通过注册的DBUS接口，获取当前消息CODE，判断需要跳转到详情界面的画面
    //! \param msgCode 消息码
    //!
    Q_SCRIPTABLE void slotJumpProcessWidget(const QString &msgCode);

    //!
    //! \brief slotRaiseWindow 窗口置顶显示
    //!
    Q_SCRIPTABLE void slotRaiseWindow();
};

#endif // DBUSFORSYSTEMOMONITORPLUGINSERVCE_H
