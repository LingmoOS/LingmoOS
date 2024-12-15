// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QSharedPointer>

#include "x11window.h"
#include "x11utils.h"
#include "qobjectdefs.h"
#include <xcb/xcb.h>
#include <unistd.h>
#include <err.h>
#include <sys/types.h>

namespace dock {
class XcbEventFilter: public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;
};

class XcbGetInfo : public QObject
{
    Q_OBJECT
Q_SIGNALS:
    void eventFilterWindowCreated(xcb_create_notify_event_t *);
    void eventFilterWindowDestroyed(xcb_destroy_notify_event_t *);
    void windowEnterChanged(xcb_window_t window);
    void windowLeaveChanged(xcb_window_t window);
    void windowPropertyChanged(xcb_window_t window,xcb_atom_t atom);

    void windowLeaveChangedInactiveName(xcb_window_t window,const QString &name);
    void windowEnterChangedActiveName(xcb_window_t window,const QString &name);

    void windowDestroyChanged(uint id);
    void windowInfoChangedForeground(const QString &name, uint id);
    void windowInfoChangedBackground(const QString &name, uint id);

public Q_SLOTS:
    void handleCreateNotifyEvent(xcb_create_notify_event_t *e);
    void handleDestroyNotifyEvent(xcb_destroy_notify_event_t *e);
    void handlePropertyNotifyEvent(xcb_window_t window,xcb_atom_t atom);

    void handleEnterEvent(xcb_window_t window);
    void handleLeaveEvent(xcb_window_t window);

public:
    void saveDestroyWindow(xcb_window_t window, char *name);
    XcbGetInfo();
    Q_INVOKABLE void getAllOpenedWiondws();
    void addWindows(xcb_window_t window);
    Q_INVOKABLE int startGetinfo();
private:
    QScopedPointer<XcbEventFilter> m_xcbEventGet;
    QHash<xcb_window_t, QSharedPointer<X11Window>> m_windows;
    QList<xcb_window_t> m_allOpenWindows;
};

}
