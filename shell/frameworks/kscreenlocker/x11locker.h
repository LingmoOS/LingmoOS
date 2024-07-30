/*
SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
SPDX-FileCopyrightText: 2002 Luboš Luňák <l.lunak@kde.org>
SPDX-FileCopyrightText: 2003 Oswald Buddenhagen <ossi@kde.org>
SPDX-FileCopyrightText: 2008 Chani Armitage <chanika@gmail.com>
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "abstractlocker.h"

#include <QAbstractNativeEventFilter>
#include <X11/Xlib.h>
#include <fixx11h.h>

namespace ScreenLocker
{
class AbstractLocker;

class X11Locker : public AbstractLocker, public QAbstractNativeEventFilter
{
    Q_OBJECT
public:
    explicit X11Locker(QObject *parent = nullptr);
    ~X11Locker() override;

    void showLockWindow() override;
    void hideLockWindow() override;

    void addAllowedWindow(quint32 window) override;

    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

private Q_SLOTS:
    void updateGeo();

private:
    void initialize();
    void saveVRoot();
    void setVRoot(Window win, Window vr);
    void removeVRoot(Window win);
    int findWindowInfo(Window w);
    void fakeFocusIn(WId window);
    void stayOnTop() override;
    struct WindowInfo {
        Window window;
        bool viewable;
    };
    QList<WindowInfo> m_windowInfo;
    QList<WId> m_lockWindows;
    QList<quint32> m_allowedWindows;
    WId m_focusedLockWindow;
};
}
