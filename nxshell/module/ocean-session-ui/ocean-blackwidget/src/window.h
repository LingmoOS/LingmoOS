// SPDX-FileCopyrightText: 2015 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QDBusAbstractAdaptor>
#include <QMouseEvent>
#include <QTimer>

const int quitInterval = 10000;

class Window : public QWidget
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = nullptr);
    ~Window();

    void paintBackground(bool show = true);
    void setupSize();
    void setupUI();
    void raiseWindow();
    void setTimer(int interval = quitInterval);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);

public Q_SLOTS:
    void onNameLost(QString name);

private:
    int m_clickCount;
    QTimer *m_timer;
    bool m_gravityRotateBlackEnabled;
};

class BlackWidgetAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.ocean.BlackScreen1")

public:
    explicit BlackWidgetAdaptor(Window * parent);
    ~BlackWidgetAdaptor();
    Window *parent() const;

public slots:
    void Raise();
    void Quit();
    void setActive(bool visible);
    void quitDBusService();
    bool blackScreenVisible();
    void releaseGrabDevices();
    void releaseGrabDevicesHideBlack();
};

#endif // WINDOW_H
