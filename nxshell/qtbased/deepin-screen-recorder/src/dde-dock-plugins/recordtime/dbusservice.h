// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSSERVICE_H
#define DBUSSERVICE_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>

class DBusService : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder.time")

public:
    explicit DBusService(QObject *parent = nullptr);

public slots:
    /**
     * @brief onStart:主程序通知插件开始计时
     * @return
     */
    Q_SCRIPTABLE bool onStart();

    /**
     * @brief onStop:主程序通知插件结束计时
     * @return
     */
    Q_SCRIPTABLE bool onStop();

    /**
     * @brief onRecording:主程序通知插件正在进行录屏
     * @return
     */
    Q_SCRIPTABLE bool onRecording();

    /**
     * @brief onRecording:主程序通知插件暂停录屏
     * @return
     */
    Q_SCRIPTABLE bool onPause();
signals:
    void start();
    void stop();
    void recording();
    void pause();
};

#endif // DBUSSERVICE_H
