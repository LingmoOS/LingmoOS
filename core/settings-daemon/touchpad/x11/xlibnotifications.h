/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLIBNOTIFICATIONS_H
#define XLIBNOTIFICATIONS_H

#include <QSocketNotifier>

#include <X11/Xlib.h>
// Undefine X11 macros that conflict with Qt6 enum values
#undef None
#undef Bool
#undef Status
#undef Success
#undef Above
#undef Always
#undef Below
#undef Bottom
#undef Top
#undef Unsorted
#undef Normal
#undef Index

#include "x11utils.h"
#include <xcb/xcb.h>

class XlibNotifications : public QObject
{
    Q_OBJECT
public:
    XlibNotifications(Display *display, int device);
    ~XlibNotifications();

Q_SIGNALS:
    void propertyChanged(xcb_atom_t);
    void devicePlugged(int);
    void touchpadDetached();

private Q_SLOTS:
    void processEvents();

private:
    void processEvent(XEvent *);

    Display *m_display;
    xcb_connection_t *m_connection;
    QSocketNotifier *m_notifier;
    xcb_window_t m_inputWindow;
    uint8_t m_inputOpcode;
    int m_device;
};

#endif // XLIBNOTIFICATIONS_H
