// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"
#include "dsutility.h"

#include <QObject>
#include <QWindow>
#include <QGuiApplication>
#include <QPointer>

DS_BEGIN_NAMESPACE

class X11Utility : public Utility
{
public:
    explicit X11Utility();
    _XDisplay *getDisplay();

    void deliverMouseEvent(uint8_t qMouseButton);

    bool grabKeyboard(QWindow *target, bool grab = true) override;
    bool grabMouse(QWindow *target, bool grab = true) override;
private:
    _XDisplay *m_display = nullptr;
};

class MouseGrabEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit MouseGrabEventFilter(QWindow *target);

    bool tryGrabMouse();
    QWindow *mainWindow() const;
    bool isMainWindow() const;
    static QWindow *mainWindow(QWindow *target);
    static bool isMainWindow(QWindow *target);
    static bool isMatchingWindow(QWindow *target);
    void closeAllWindow();
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void outsideMousePressed();
private:
    void mousePressEvent(QMouseEvent *e);
    bool trySelectGrabWindow(QMouseEvent *e);

    QPointer<QWindow> m_target;
};

DS_END_NAMESPACE
