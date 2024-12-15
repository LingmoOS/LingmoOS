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

static const QEvent::Type kMonitorStartEventType = static_cast<QEvent::Type>(QEvent::User + 1);
static const QEvent::Type kNetifStartEventType = static_cast<QEvent::Type>(QEvent::User + 2);
class MonitorStartEvent : public QEvent
{
public:
    explicit MonitorStartEvent()
        : QEvent(kMonitorStartEventType)
    {
    }
    virtual ~MonitorStartEvent();
};

class NetifStartEvent : public QEvent
{
public:
    explicit NetifStartEvent()
        : QEvent(kNetifStartEventType)
    {
    }
    virtual ~NetifStartEvent();
};

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

    void setMainWindow(MainWindow *mw);
    MainWindow *mainWindow();

signals:
    void backgroundTaskStateChanged(Application::TaskState state);

protected:
    bool event(QEvent *event) override;

private:
    MainWindow *m_mainWindow {};
};

inline void Application::setMainWindow(MainWindow *mw)
{
    m_mainWindow = mw;
}

inline MainWindow *Application::mainWindow()
{
    return m_mainWindow;
}

#endif // APPLICATION_H
