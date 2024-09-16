// SPDX-FileCopyrightText: 2016 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef WINDOWEVENTHOOK_H
#define WINDOWEVENTHOOK_H

#include "qxcbwindow.h"

#include "global.h"

DPP_BEGIN_NAMESPACE

class WindowEventHook
{
public:
    static void init(QXcbWindow *window, bool redirectContent);

    static void handleConfigureNotifyEvent(QXcbWindow *window, const xcb_configure_notify_event_t *);
    static void handleMapNotifyEvent(QXcbWindow *window, const xcb_map_notify_event_t *);

    static void handleClientMessageEvent(QXcbWindow *window, const xcb_client_message_event_t *event);
    static void handleFocusInEvent(QXcbWindow *window, const xcb_focus_in_event_t *event);
    static void handleFocusOutEvent(QXcbWindow *window, const xcb_focus_out_event_t *event);
    static void handlePropertyNotifyEvent(QXcbWindowEventListener *el, const xcb_property_notify_event_t *event);
#ifdef XCB_USE_XINPUT22
    static void handleXIEnterLeave(QXcbWindow *window, xcb_ge_event_t *event);
#endif
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
    static void windowEvent(QPlatformWindow *window, QEvent *event);
#else
    static bool windowEvent(QXcbWindow *window, QEvent *event);
#endif
};

DPP_END_NAMESPACE

#endif // WINDOWEVENTHOOK_H
