// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H
#define APPLICATION_H

#include <DApplication>

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

#if defined(gApp)
#undef gApp
#endif
#define gApp (qobject_cast<Application *>(Application::instance()))

class MainWindow;
class Application : public DApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);

    enum TaskState {
        kTaskStarted,
        kTaskRunning,
        kTaskFinished
    };
    Q_ENUM(Application::TaskState)

    inline void setMainWindow(MainWindow *mw)
    {
        m_mainWindow = mw;
    }
    inline MainWindow *mainWindow()
    {
        Q_ASSERT(m_mainWindow != nullptr);
        return m_mainWindow;
    }

signals:
    void backgroundTaskStateChanged(Application::TaskState state);

private:
    MainWindow *m_mainWindow {};
};

#endif // APPLICATION_H
