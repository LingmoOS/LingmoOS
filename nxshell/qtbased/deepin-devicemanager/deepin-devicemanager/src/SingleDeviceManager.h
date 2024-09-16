// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEDEVICEMANAGER_H
#define SINGLEDEVICEMANAGER_H

#include "MainWindow.h"

#include <DApplication>
#include <DMainWindow>

#include <QCommandLineParser>
#include <QString>

DWIDGET_USE_NAMESPACE

class SingleDeviceManager : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.DeviceManagerNotify")

public:
    enum TaskState {
        kTaskStarted,
        kTaskRunning,
        kTaskFinished
    };
    Q_ENUM(SingleDeviceManager::TaskState)

    explicit SingleDeviceManager(int &argc, char **argv);

    void activateWindow();

    bool parseCmdLine();

signals:
    void backgroundTaskStateChanged(SingleDeviceManager::TaskState state);

public slots:
    Q_SCRIPTABLE void startDeviceManager(QString pageDescription);

private:
    QString m_PageDescription;
    QScopedPointer<MainWindow> m_qspMainWnd;  // MainWindow ptr
};

#endif // SINGLEDEVICEMANAGER_H
