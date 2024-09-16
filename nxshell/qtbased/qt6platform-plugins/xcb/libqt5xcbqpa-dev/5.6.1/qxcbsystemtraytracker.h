// Copyright (C) 2016 The Qt Company Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QXCBSYSTEMTRAYTRACKER_H
#define QXCBSYSTEMTRAYTRACKER_H

#include "qxcbconnection.h"

#include <xcb/xcb.h>

QT_BEGIN_NAMESPACE

class QXcbConnection;
class QScreen;

class QXcbSystemTrayTracker : public QObject, public QXcbWindowEventListener
{
    Q_OBJECT
public:
    static QXcbSystemTrayTracker *create(QXcbConnection *connection);

    xcb_window_t trayWindow();
    void requestSystemTrayWindowDock(xcb_window_t window) const;
    QRect systemTrayWindowGlobalGeometry(xcb_window_t window) const;

    void notifyManagerClientMessageEvent(const xcb_client_message_event_t *);

    void handleDestroyNotifyEvent(const xcb_destroy_notify_event_t *) Q_DECL_OVERRIDE;

    bool visualHasAlphaChannel();
signals:
    void systemTrayWindowChanged(QScreen *screen);

private:
    explicit QXcbSystemTrayTracker(QXcbConnection *connection,
                                   xcb_atom_t trayAtom,
                                   xcb_atom_t selection);
    static xcb_window_t locateTrayWindow(const QXcbConnection *connection, xcb_atom_t selection);
    void emitSystemTrayWindowChanged();

    const xcb_atom_t m_selection;
    const xcb_atom_t m_trayAtom;
    QXcbConnection *m_connection;
    xcb_window_t m_trayWindow;
};

QT_END_NAMESPACE

#endif // QXCBSYSTEMTRAYTRACKER_H
